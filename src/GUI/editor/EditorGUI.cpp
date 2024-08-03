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
#include "../../core/utils/gl_utils.hpp"
#include "../../assets/BasePart.hpp"


EditorGUI::EditorGUI(){
    m_fb_update = true;
    m_init = false;
}


const float c[4] = {.85f, .85f, .85f, 1.f};


EditorGUI::EditorGUI(const BaseApp* app, const FontAtlas* atlas){
    assert(app);
    assert(atlas);

    m_app = app;
    m_render_context = m_app->getRenderContext();
    m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
    m_fb_update = true;
    m_init = true;
    m_font_atlas = atlas;
    m_input = m_app->getInput();
    m_tab_option = TAB_OPTION_PARTS;
    m_input->getMousePos(m_mouse_y, m_mouse_x);
    m_mouse_y = m_fb_height - m_mouse_y;
    m_symmetric_sides = 1;
    m_max_symmetric_sides = 8;
    m_radial_align = true;
    strcpy(m_vessel_name, "Unnamed vessel");
    m_action = EDITOR_ACTION_NONE;
    m_master_parts_list = nullptr;

    m_main_text.reset(new Text2D(m_fb_width, m_fb_height, m_font_atlas, m_render_context));

    // texture atlas loading test
    int n;
    unsigned char* image_data = stbi_load("../data/editor_atlas.png", &m_tex_size_x, &m_tex_size_y, &n, 4);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_tex_size_x, m_tex_size_y,
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


void EditorGUI::onFramebufferSizeUpdate(){
    m_fb_update = true;
}


void EditorGUI::updateBuffers(){

}


void EditorGUI::setButtonColor(float r ,float g, float b, float a, GLintptr offset){
    UNUSED(r);
    UNUSED(g);
  UNUSED(b);  
  UNUSED(a);
  UNUSED(offset);

}


void EditorGUI::updateTabsColor(){
    
}


void EditorGUI::updateDeleteArea(){

}


void EditorGUI::updateTopButtons(){
 

}


void EditorGUI::updateButtons(){ // used to update button colors
    //updateTabsColor();
    //updateDeleteArea();
    //updateTopButtons();
}


void EditorGUI::render(){

}


int EditorGUI::update(){
    return action;
}


void EditorGUI::setMasterPartList(const std::unordered_map<std::uint32_t,
                                  std::unique_ptr<BasePart>>* master_parts_list){
    m_master_parts_list = master_parts_list;
}


const std::unique_ptr<BasePart>* EditorGUI::getPickedObject() const{
    return m_parts_panel->getPickedObject();
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


void EditorGUI::drawLeftPanel(){
    int fb_x, fb_y;
    m_app->getWindowHandler()->getFramebufferSize(fb_x, fb_y);
    static bool window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
                             | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus
                             | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove
                             | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoResize;

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
        if(ImGui::BeginTabItem("Parts") && m_master_parts_list){
            ImGui::Dummy(ImVec2(0.5, 0)); ImGui::SameLine();

            ImGui::BeginChildFrame(102, ImVec2(290, fb_y * .5), window_flags);
            ImGui::PushItemWidth(285);

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

                ImGui::PushID(it->second->getBaseId());

                pos = ImGui::GetCursorScreenPos();
                ImGui::Selectable("##give_id", i%2, 0, ImVec2(285, 35));
                
                ImVec2 b_im_size = ImVec2(35, 35);
                ImVec2 uv0 = ImVec2(0.8203125, 0.41015625);
                ImVec2 uv1 = ImVec2(0.95703125, 0.546875);
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

            ImGui::PopItemWidth();
            ImGui::EndChildFrame();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Staging")){
            ImGui::Text("Stages");
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Other")){
            ImGui::Text("Other");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

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

    ImGui::PopStyleVar();

    ImGui::End();

}