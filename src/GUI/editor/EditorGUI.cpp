#include <cstring>
#include <cassert>

#include <stb/stb_image.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "PartsPanelGUI.hpp"
#include "StagingPanelGUI.hpp"
#include "EditorGUI.hpp"
#include "../Sprite.hpp"
#include "../FontAtlas.hpp"
#include "../Text2D.hpp"
#include "../../core/BaseApp.hpp"
#include "../../core/RenderContext.hpp"
#include "../../core/Input.hpp"
#include "../../core/log.hpp"
#include "../../core/WindowHandler.hpp"
#include "../../core/AssetManager.hpp"
#include "../../core/utils/gl_utils.hpp"
#include "../../assets/BasePart.hpp"
#include "../../assets/Vessel.hpp"


EditorGUI::EditorGUI(){
    m_init = false;
}


const float c[4] = {.85f, .85f, .85f, 1.f};


EditorGUI::EditorGUI(const BaseApp* app, const FontAtlas* atlas){
    assert(app);
    assert(atlas);

    float fb_width, fb_height;

    m_app = app;
    m_render_context = m_app->getRenderContext();
    m_render_context->getDefaultFbSize(fb_width, fb_height);
    m_init = true;
    m_font_atlas = atlas;
    m_input = m_app->getInput();
    m_symmetric_sides = 1;
    m_max_symmetric_sides = 8;
    m_radial_align = true;
    strcpy(m_vessel_name, "Unnamed vessel");
    m_action = EDITOR_ACTION_NONE;
    m_master_parts_list = nullptr;
    m_picked_object = 0;
    m_action_swap = {-1, -1};
    m_highlight_part = nullptr;
    m_new_stage_pos = 0;

    m_main_text.reset(new Text2D(fb_width, fb_height, m_font_atlas, m_render_context));

    // texture atlas loading test
    int n, x, y;
    unsigned char* image_data = stbi_load("../data/editor_atlas.png", &x, &y, &n, 4);
    if(!image_data) {
        std::cerr << "EditorGUI::EditorGUI - could not load GUI texture atlas" << std::endl;
        log("EditorGUI::EditorGUI - could not load GUI texture atlas");

        m_tex_size_x = 0;
        m_tex_size_y = 0;
    }
    else{
        glGenTextures(1, &m_texture_atlas);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture_atlas);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    stbi_image_free(image_data);
}


EditorGUI::~EditorGUI(){
    if(m_init)
        glDeleteTextures(1, &m_texture_atlas);
}


int EditorGUI::update(){
    int action = m_action;
    m_action = EDITOR_ACTION_NONE;

    return action;
}


void EditorGUI::setMasterPartList(const std::unordered_map<std::uint32_t,
                                  std::unique_ptr<BasePart>>* master_parts_list){
    m_master_parts_list = master_parts_list;
}


const std::unique_ptr<BasePart>* EditorGUI::getPickedObject() const{
    return &m_master_parts_list->at(m_picked_object);
}


void EditorGUI::setSymmetrySides(int sides){
    m_symmetric_sides = sides;
}


int EditorGUI::getSymmetrySides() const{
    return m_symmetric_sides;
}


void EditorGUI::setRadialAlign(bool to_what){
    m_radial_align = to_what;
}


bool EditorGUI::getRadialAlign() const{
    return m_radial_align;
}


void EditorGUI::renderImGUI(){
    drawTaskBar();
    drawLeftPanel();
}


void EditorGUI::onFramebufferSizeUpdate(){
}


void EditorGUI::render(){
}


// global for all the windows
static bool window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
                         | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus
                         | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove
                         | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoResize;

void EditorGUI::drawPartsTab(){
    int fb_x, fb_y;
    m_app->getWindowHandler()->getFramebufferSize(fb_x, fb_y);
    ImVec2 uv0, uv1, b_im_size;

    ImGui::Dummy(ImVec2(1, 0)); ImGui::SameLine();
    ImGui::BeginChildFrame(102, ImVec2(290, 100), window_flags);
    ImGui::PushItemWidth(285);

    const char* types[] = {"All", "Engines", "Tanks", "Others"};
    static int item_current = 0;
    ImGui::Text("Show type:");
    ImGui::Combo("##part_type", &item_current, types, IM_ARRAYSIZE(types));

    const char* sort_types[] = {"Alphabetical", "Cost", "Others"};
    static int sort_type = 0;
    ImGui::Text("Sort by:");
    ImGui::Combo("##sort_by", &sort_type, sort_types, IM_ARRAYSIZE(sort_types));
    ImGui::PopItemWidth();
    ImGui::EndChildFrame();

    ImGui::Dummy(ImVec2(1, 0)); ImGui::SameLine();
    ImGui::BeginChildFrame(104, ImVec2(290, fb_y * 0.5), window_flags);
    std::unordered_map<std::uint32_t, std::unique_ptr<BasePart>>::const_iterator it;
    uint i = 0;
    for(it = m_master_parts_list->begin(); it != m_master_parts_list->end(); it++){
        std::string part_name;
        i++;

        ImVec2 pos = ImGui::GetCursorScreenPos();
        if(i != 1){
            pos.y -= 8;
            ImGui::SetCursorScreenPos(pos);
        }
        else{
            pos.y += 8;
            ImGui::SetCursorScreenPos(pos);
        }

        ImGui::PushID(it->second->getBaseId());

        pos = ImGui::GetCursorScreenPos();
        if(ImGui::Selectable("##give_id", i%2, 0, ImVec2(285, 35))){
            m_picked_object = it->first;
            m_action = EDITOR_ACTION_OBJECT_PICK;
        }
        
        b_im_size = ImVec2(35, 35);
        uv0 = ImVec2(0.8203125, 0.41015625);
        uv1 = ImVec2(0.95703125, 0.546875);
        ImGui::SetCursorScreenPos(pos);
        ImGui::Image((ImTextureID*)(intptr_t)m_texture_atlas, b_im_size, uv0, uv1);

        ImGui::SameLine();
        pos = ImGui::GetCursorScreenPos();
        pos.y += 8;
        ImGui::SetCursorScreenPos(pos);
        it->second->getFancyName(part_name);
        ImGui::Text(part_name.c_str());

        ImGui::PopID();
    }
    ImGui::EndChildFrame();
    ImGui::EndTabItem();
}


void EditorGUI::drawStagingTab(){
    int fb_x, fb_y;
    m_app->getWindowHandler()->getFramebufferSize(fb_x, fb_y);
    ImVec2 uv0, uv1, b_im_size;
    Vessel* editor_vessel = m_app->getAssetManager()->m_editor_vessel.get();

    ImGui::Dummy(ImVec2(1, 0)); ImGui::SameLine();
    ImGui::BeginChildFrame(102, ImVec2(290, fb_y * 0.5), window_flags);
    if(editor_vessel){
        vessel_stages* stages = editor_vessel->getStages();
        int index_1 = -1, index_2 = -1;
        for(uint i=0; i < stages->size(); i++){
            std::string stage_name = "Stage ";
            stage_name += std::to_string(i);

            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImGui::Text(stage_name.c_str());

            if(!stages->at(i).size()){
                ImGui::SetCursorScreenPos(pos);
                ImGui::Dummy(ImVec2(225, 0)); ImGui::SameLine();
                if(ImGui::Button(std::string(std::string(" - ##") + std::to_string(i)).c_str())){
                    m_action = EDITOR_ACTION_REMOVE_STAGE;
                    m_new_stage_pos = i;  // we use the same name wtf
                }
            }

            ImGui::SetCursorScreenPos(pos);
            ImGui::Dummy(ImVec2(250, 0)); ImGui::SameLine();
            if(ImGui::Button(std::string(std::string(" + ##") + std::to_string(i)).c_str())){
                m_action = EDITOR_ACTION_ADD_STAGE;
                m_new_stage_pos = i;
            }

            for(uint j=0; j < stages->at(i).size(); j++){
                const stage_action& action = stages->at(i).at(j);

                ImGui::Dummy(ImVec2(35, 0)); ImGui::SameLine();
                std::string action_str = "    ";
                action.part->getFancyName(action_str);
                action_str += " action " + std::to_string(action.action);
                action_str +=  + "##" + std::string(std::to_string(
                                  action.part->getUniqueId())) + 
                                  std::to_string(action.action);
                ImGui::Selectable(action_str.c_str());

                if(ImGui::IsItemHovered())
                    m_highlight_part = action.part;

                if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)){
                    m_action_swap = {i, j};
                    ImGui::SetDragDropPayload("STAGE_ACTION_MOVE", &m_action_swap,
                                              sizeof(m_action_swap));
                    ImGui::EndDragDropSource();
                }
            }

            ImVec2 pos2 = ImGui::GetCursorScreenPos();
            ImGui::SetCursorScreenPos(pos);
            ImGui::Selectable(std::string("##stage_" + std::to_string(i)).c_str() , 
                false, 0, ImVec2(285, pos2.y - pos.y));

            if(ImGui::BeginDragDropTarget()){
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("STAGE_ACTION_MOVE");
                if(payload){
                    int s_i = std::get<0>(m_action_swap), s_j = std::get<1>(m_action_swap);
                    stages->at(i).push_back(stages->at(s_i).at(s_j));
                    stages->at(s_i).erase(stages->at(s_i).begin() + s_j);
                }

                ImGui::EndDragDropTarget();
            }

            /*if(ImGui::IsItemActive() && !ImGui::IsItemHovered()){
                int n_next = i + (ImGui::GetMouseDragDelta(0).y < 0.f ? -1 : 1);
                if(n_next >= 0 && n_next < (int)stages->size()){
                    index_1 = n_next;
                    index_2 = i;
                    ImGui::ResetMouseDragDelta();
                }
            }*/
        }
        if(index_1 != -1 && index_2 != -1)
            std::swap(stages->at(index_1), stages->at(index_2));
    }
    ImGui::EndChildFrame();
    ImGui::EndTabItem();
}


void EditorGUI::drawLeftPanel(){
    int fb_x, fb_y;
    m_app->getWindowHandler()->getFramebufferSize(fb_x, fb_y);
    ImVec2 uv0, uv1, b_im_size;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

    ImGui::SetNextWindowSize(ImVec2(305, fb_y - 55));
    ImGui::SetNextWindowPos(ImVec2(0, 55));
    ImGui::Begin("LEFT_PANEL", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    ImGui::Dummy(ImVec2(0.5, 0)); ImGui::SameLine();
    ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
    ImGui::PushItemWidth(295);
    if (ImGui::BeginTabBar("taskbar", tab_bar_flags)){
        m_highlight_part = nullptr; // reset highlighted part here
        /*ImVec2 pos = ImGui::GetCursorScreenPos();
        pos.x += 1;
        ImGui::SetCursorScreenPos(pos);*/
        if(ImGui::BeginTabItem("Parts") && m_master_parts_list)
            drawPartsTab();

        if (ImGui::BeginTabItem("Staging"))
            drawStagingTab();

        if (ImGui::BeginTabItem("Other")){
            ImGui::Text("Other");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImVec2 pos;
    ImGui::Dummy(ImVec2(1, 0)); ImGui::SameLine();

    ImGui::BeginChildFrame(103, ImVec2(290, 85), window_flags);
    b_im_size = ImVec2(240, 75);
    uv0 = ImVec2(0.0, 0.0);
    uv1 = ImVec2(0.9375, 0.2734375);
    pos = ImGui::GetCursorScreenPos();
    pos.x += 20;
    ImGui::SetCursorScreenPos(pos);
    if(ImGui::ImageButton((ImTextureID*)(intptr_t)m_texture_atlas,
                           b_im_size, uv0, uv1, 0, ImVec4(0., 0., 0., 0)))
        m_action = EDITOR_ACTION_DELETE;

    ImGui::EndChildFrame();

    ImGui::End();
}


void EditorGUI::drawTaskBar(){
    int fb_x, fb_y;
    m_app->getWindowHandler()->getFramebufferSize(fb_x, fb_y);
    static bool window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
                             | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus
                             | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove
                             | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoResize;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.f);

    ImGui::SetNextWindowSize(ImVec2(fb_x, 55));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin("TOP_BAR", nullptr, window_flags);
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    ImGui::Dummy(ImVec2(0, 3));
    ImGui::Dummy(ImVec2(0, 0)); ImGui::SameLine();

    // style_var for the buttons
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.f);

    // minus button
    ImVec2 b_im_size = ImVec2(35, 35);
    ImVec2 uv0 = ImVec2(0.1367f * 4, 0.2734f + 0.1367f * 1);
    ImVec2 uv1 = ImVec2(0.1367f * 5, 0.2734f + 0.1367f * 2);
    if(ImGui::ImageButton((ImTextureID*)(intptr_t)m_texture_atlas, b_im_size, uv0, uv1)){
        m_symmetric_sides--;
        if(m_symmetric_sides <= 0)
            m_symmetric_sides = 1;
        m_action = EDITOR_ACTION_SYMMETRY_SIDES;
    }

    ImGui::SameLine();

    // symmetry sides image
    float row = m_symmetric_sides / 8;
    float col = ((m_symmetric_sides - 1) % 7) + 1;
    uv0 = ImVec2(0.1367f * (col - 1), 0.2734f + 0.1367f * row);
    uv1 = ImVec2(0.1367f * col, 0.2734f + 0.1367f * (row + 1));
    ImGui::Image((ImTextureID*)(intptr_t)m_texture_atlas, b_im_size, uv0, uv1);

    ImGui::SameLine();

    // plus button
    uv0 = ImVec2(0.1367f * 5, 0.2734f + 0.1367f * 1);
    uv1 = ImVec2(0.1367f * 6, 0.2734f + 0.1367f * 2);
    ImGui::PushID("plus_sides"); // might not be neccessary for more recent dear imgui versions
    if(ImGui::ImageButton((ImTextureID*)(intptr_t)m_texture_atlas, b_im_size, uv0, uv1)){
        m_symmetric_sides++;
        if(m_symmetric_sides > m_max_symmetric_sides)
            m_symmetric_sides = m_max_symmetric_sides;
        m_action = EDITOR_ACTION_SYMMETRY_SIDES;
    }
    ImGui::PopID();

    ImGui::SameLine();

    // align type button
    row = m_radial_align ? 2.0f : 1.0f;
    uv0 = ImVec2(0.1367f * row, 0.4101f);
    uv1 = ImVec2(0.1367f * (row + 1), 0.5468f);
    ImGui::PushID("symmetry_type");
    if(ImGui::ImageButton((ImTextureID*)(intptr_t)m_texture_atlas, b_im_size, uv0, uv1)){
        m_radial_align = !m_radial_align;
        m_action = EDITOR_ACTION_TOGGLE_ALIGN;
    }
    ImGui::PopID();

    ImGui::SameLine();

    // trash button
    row = m_radial_align ? 2.0f : 1.0f;
    uv0 = ImVec2(0.41015625f, 0.41015625f);
    uv1 = ImVec2(0.546875f, 0.546875f);
    ImGui::PushID("clear_scene");
    if(ImGui::ImageButton((ImTextureID*)(intptr_t)m_texture_atlas, b_im_size, uv0, uv1))
        m_action = EDITOR_ACTION_CLEAR_SCENE;
    ImGui::PopID();

    ImGui::SameLine();

    // vessel name
    ImGui::Dummy(ImVec2(100, 0)); ImGui::SameLine();
    // to better adjust
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::BeginChildFrame(101, ImVec2(210, 45), window_flags);
    ImGui::Dummy(ImVec2(0, 3));
    ImGui::PushItemWidth(200);
    if(ImGui::InputText("##vessel_name", m_vessel_name, 32))
        m_action = EDITOR_ACTION_CHANGE_NAME;
    ImGui::PopItemWidth();
    ImGui::EndChildFrame();

    ImGui::SameLine();

    // launch button
    ImGui::SetCursorPosX(fb_x - 110);
    uv0 = ImVec2(0.f, 0.546875f);
    uv1 = ImVec2(0.13671875f, 0.68359375f);
    ImGui::PushID("launch");
    if(ImGui::ImageButton((ImTextureID*)(intptr_t)m_texture_atlas, b_im_size, uv0, uv1))
        m_action = EDITOR_ACTION_LAUNCH;
    ImGui::PopID();

    ImGui::SameLine();

    // launch button
    uv0 = ImVec2(0.13671875f, 0.546875f);
    uv1 = ImVec2(0.2734375f, 0.68359375f);
    ImGui::PushID("exit");
    if(ImGui::ImageButton((ImTextureID*)(intptr_t)m_texture_atlas, b_im_size, uv0, uv1))
        m_action = EDITOR_ACTION_EXIT;
    ImGui::PopID();

    // end
    ImGui::PopStyleVar();

    ImGui::End();

}


BasePart* EditorGUI::getHighlightedPart() const{
    return m_highlight_part;
}


uint EditorGUI::getNewStagePos() const{
    return m_new_stage_pos;
}
