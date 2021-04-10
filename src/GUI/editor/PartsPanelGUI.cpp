#include <cstdlib>
#include <cstring>

#include "PartsPanelGUI.hpp"
#include "../FontAtlas.hpp"
#include "../Text2D.hpp"
#include "../../assets/BasePart.hpp"
#include "../../core/RenderContext.hpp"
#include "../../core/Input.hpp"


PartsPanelGUI::PartsPanelGUI(float fb_width, float fb_height, const FontAtlas* atlas, const RenderContext* render_context, const Input* input){
    m_fb_width = fb_width;
    m_fb_height = fb_height;
    m_master_parts_list = nullptr;
    m_font_atlas = atlas;
    m_render_context = render_context;
    m_input = input;
    m_num_vert = 0;
    m_item_mouseover = ITEM_MOUSEOVER_NONE;
    m_last_item_colored = ITEM_MOUSEOVER_NONE;
    m_panel_scroll = math::vec2(0.0, 0.0);
    m_picked_part = nullptr;

    m_projection_location = m_render_context->getUniformLocation(SHADER_GUI, "projection");
    m_disp_location = m_render_context->getUniformLocation(SHADER_GUI, "disp");

    m_text_projection_location = m_render_context->getUniformLocation(SHADER_TEXT, "projection");

    m_projection = math::orthographic(fb_width, 0, fb_height, 0, 1.0f , -1.0f);

    color c{1., 1., 1.};
    m_text.reset(new Text2D(fb_width, fb_height, c, m_font_atlas, render_context));

    glGenFramebuffers(1, &m_fb);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fb);
    glGenTextures(1, &m_tex);

    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb_width, fb_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex, 0);

    GLenum draw_bufs[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_bufs);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // part list panel
    glGenVertexArrays(1, &m_vao);
    render_context->bindVao(m_vao);

    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glVertexAttribPointer(0, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_vbo_clr);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_clr);
    glVertexAttribPointer(1, 4,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glVertexAttribPointer(2, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);
}


PartsPanelGUI::~PartsPanelGUI(){
    glDeleteBuffers(1, &m_vbo_vert);
    glDeleteBuffers(1, &m_vbo_tex);
    glDeleteBuffers(1, &m_vbo_clr);
    glDeleteVertexArrays(1, &m_vao);
    glDeleteTextures(1, &m_tex);
    glDeleteFramebuffers(1, &m_fb);
}


void PartsPanelGUI::setMasterPartList(const std::unordered_map<std::uint32_t, std::unique_ptr<BasePart>>* master_parts_list){
    int i = 0;
    m_master_parts_list = master_parts_list;
    m_item_to_key.clear();

    std::unordered_map<std::uint32_t, std::unique_ptr<BasePart>>::const_iterator it;
    for(it = m_master_parts_list->begin(); it != m_master_parts_list->end(); it++){
        std::string name;
        wchar_t wname[STRING_MAX_LEN];

        m_item_to_key.insert(std::pair<int,std::uint32_t>(i, it->first));

        it->second->getFancyName(name);
        std::mbstowcs(wname, name.c_str(), STRING_MAX_LEN);
        m_text->addString(wname, 10., 20. + (float)i*20., 1, STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);

        i++;
    }

    m_num_vert = m_master_parts_list->size() * 6;
    updateBuffers();
}


void PartsPanelGUI::render(){
    float fb_width, fb_height;
    math::mat4 projection;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fb);
    glViewport(0, 0, m_fb_width, m_fb_height);
    glClearColor(0.25, 0.25, 0.25, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_render_context->useProgram(SHADER_GUI);
    glUniformMatrix4fv(m_projection_location, 1, GL_FALSE, m_projection.m);
    glUniform2fv(m_disp_location, 1, m_panel_scroll.v);
    m_render_context->bindVao(m_vao);
    buttonMouseoverColor();
    glDrawArrays(GL_TRIANGLES, 0, m_num_vert);

    // restore the original framebuffer uniform
    m_render_context->getDefaultFbSize(fb_width, fb_height);
    projection = math::orthographic(fb_width, 0, fb_height, 0, 1.0f , -1.0f); // we have to compute that each frame, not ideal
    glUniformMatrix4fv(m_projection_location, 1, GL_FALSE, projection.m);
    glUniform2f(m_disp_location, 0.0f, 0.0f);

    m_render_context->useProgram(SHADER_TEXT);
    glUniformMatrix4fv(m_text_projection_location, 1, GL_FALSE, m_projection.m);
    m_text->render();
    glUniformMatrix4fv(m_text_projection_location, 1, GL_FALSE, projection.m);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, fb_width, fb_height);
}


void PartsPanelGUI::onFramebufferSizeUpdate(float fb_width, float fb_height){
    m_fb_width = fb_width;
    m_fb_height = fb_height;

    m_projection = math::orthographic((float)fb_width, 0, (float)fb_height, 0, 1.0f , -1.0f);

    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb_width, fb_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    m_text->onFramebufferSizeUpdate(fb_width, fb_height);
    updateBuffers();
    m_panel_scroll.v[1] = 0.0f;
    m_text->setDisplacement(m_panel_scroll);
}


void PartsPanelGUI::bindTexture(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex);
}


void PartsPanelGUI::updateBuffers(){
    float color;

    m_render_context->bindVao(m_vao);

    std::unique_ptr<GLfloat[]> vertex_buffer;
    std::unique_ptr<GLfloat[]> tex_coords_buffer;
    std::unique_ptr<GLfloat[]> color_buffer;
    
    vertex_buffer.reset(new GLfloat[m_num_vert * 12]);
    tex_coords_buffer.reset(new GLfloat[m_num_vert * 18]);
    color_buffer.reset(new GLfloat[m_num_vert * 24]);

    std::memset(tex_coords_buffer.get(), 0, m_num_vert * 18 * sizeof(GLfloat));
    for(uint i=0; i < m_master_parts_list->size(); i++){
        if(i & 1){
            color = ITEM_COLOR_1;
        }
        else{
            color = ITEM_COLOR_2;
        }

        vertex_buffer[i * 12] = 0.0f + LIST_MARGIN;
        vertex_buffer[i * 12 + 1] = m_fb_height - LIST_MARGIN - ITEM_SEPARATION * i;
        vertex_buffer[i * 12 + 2] = m_fb_width - LIST_MARGIN;
        vertex_buffer[i * 12 + 3] = m_fb_height - LIST_MARGIN - ITEM_SEPARATION * (i + 1);
        vertex_buffer[i * 12 + 4] = m_fb_width - LIST_MARGIN;
        vertex_buffer[i * 12 + 5] = m_fb_height - LIST_MARGIN - ITEM_SEPARATION * i;
        vertex_buffer[i * 12 + 6] = 0.0f + LIST_MARGIN;
        vertex_buffer[i * 12 + 7] = m_fb_height - LIST_MARGIN - ITEM_SEPARATION * i;
        vertex_buffer[i * 12 + 8] = 0.0f + LIST_MARGIN;
        vertex_buffer[i * 12 + 9] = m_fb_height - LIST_MARGIN - ITEM_SEPARATION * (i + 1);
        vertex_buffer[i * 12 + 10] = m_fb_width - LIST_MARGIN;
        vertex_buffer[i * 12 + 11] = m_fb_height - LIST_MARGIN - ITEM_SEPARATION * (i + 1);

        color_buffer[i * 24] = color;
        color_buffer[i * 24 + 1] = color;
        color_buffer[i * 24 + 2] = color;
        color_buffer[i * 24 + 3] = 1.0f;
        color_buffer[i * 24 + 4] = color;
        color_buffer[i * 24 + 5] = color;
        color_buffer[i * 24 + 6] = color;
        color_buffer[i * 24 + 7] = 1.0f;
        color_buffer[i * 24 + 8] = color;
        color_buffer[i * 24 + 9] = color;
        color_buffer[i * 24 + 10] = color;
        color_buffer[i * 24 + 11] = 1.0f;
        color_buffer[i * 24 + 12] = color;
        color_buffer[i * 24 + 13] = color;
        color_buffer[i * 24 + 14] = color;
        color_buffer[i * 24 + 15] = 1.0f;
        color_buffer[i * 24 + 16] = color;
        color_buffer[i * 24 + 17] = color;
        color_buffer[i * 24 + 18] = color;
        color_buffer[i * 24 + 19] = 1.0f;
        color_buffer[i * 24 + 20] = color;
        color_buffer[i * 24 + 21] = color;
        color_buffer[i * 24 + 22] = color;
        color_buffer[i * 24 + 23] = 1.0f;
    }


    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, m_num_vert * 12 * sizeof(GLfloat), vertex_buffer.get(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_clr);
    glBufferData(GL_ARRAY_BUFFER, m_num_vert * 18 * sizeof(GLfloat), color_buffer.get(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, m_num_vert * 24 * sizeof(GLfloat), tex_coords_buffer.get(), GL_STATIC_DRAW);
}


int PartsPanelGUI::update(float mouse_x, float mouse_y){
    double scx, scy;
    float disp;

    if(m_render_context->imGuiWantCaptureMouse()){
        m_item_mouseover = ITEM_MOUSEOVER_NONE;
        return PANEL_ACTION_NONE;
    }

    mouse_y = m_fb_height - mouse_y; // change the vertical origin for convenience

    m_input->getScroll(scx, scy);
    disp = m_panel_scroll.v[1] + PANEL_SCROLL_STEP * -scy;

    if(mouse_x > 0 && mouse_x < m_fb_width && mouse_y > 0 && mouse_y < m_fb_height){
        
        if(disp >= 0 && disp + m_fb_height < m_master_parts_list->size() * ITEM_SEPARATION + PANEL_SCROLL_MARGIN){
            m_panel_scroll.v[1] += PANEL_SCROLL_STEP * -scy;
            m_text->setDisplacement(m_panel_scroll);
        }

        int i = (mouse_y + m_panel_scroll.v[1] - LIST_MARGIN) / ITEM_SEPARATION;
        if((uint)i < m_master_parts_list->size()){
            m_item_mouseover = i;

            if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS){
                m_picked_part = &m_master_parts_list->at(m_item_to_key.at(i));
                return PANEL_ACTION_PICK;    
            }
            return PANEL_ACTION_NONE;
        }
        else{
            m_item_mouseover = ITEM_MOUSEOVER_NONE;
            return PANEL_ACTION_NONE;
        }
    }
    else{
        m_item_mouseover = ITEM_MOUSEOVER_NONE;
        return PANEL_ACTION_NONE;
    }
}


void PartsPanelGUI::buttonMouseoverColor(){
    // we're assuming that the vao has been already bound (from the render method)
    int temp = m_last_item_colored;
    float color;

    if(m_last_item_colored != ITEM_MOUSEOVER_NONE && m_item_mouseover != m_last_item_colored){
        GLfloat color_buffer_subdata[24];

        if(m_last_item_colored & 1){
            color = ITEM_COLOR_1;
        }
        else{
            color = ITEM_COLOR_2;
        }

        color_buffer_subdata[0] = color;
        color_buffer_subdata[1] = color;
        color_buffer_subdata[2] = color;
        color_buffer_subdata[3] = 1.0f;
        color_buffer_subdata[4] = color;
        color_buffer_subdata[5] = color;
        color_buffer_subdata[6] = color;
        color_buffer_subdata[7] = 1.0f;
        color_buffer_subdata[8] = color;
        color_buffer_subdata[9] = color;
        color_buffer_subdata[10] = color;
        color_buffer_subdata[11] = 1.0f;
        color_buffer_subdata[12] = color;
        color_buffer_subdata[13] = color;
        color_buffer_subdata[14] = color;
        color_buffer_subdata[15] = 1.0f;
        color_buffer_subdata[16] = color;
        color_buffer_subdata[17] = color;
        color_buffer_subdata[18] = color;
        color_buffer_subdata[19] = 1.0f;
        color_buffer_subdata[20] = color;
        color_buffer_subdata[21] = color;
        color_buffer_subdata[22] = color;
        color_buffer_subdata[23] = 1.0f;

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo_clr);
        glBufferSubData(GL_ARRAY_BUFFER, m_last_item_colored * 24 * sizeof(GLfloat), 24 * sizeof(GLfloat), color_buffer_subdata);
        m_last_item_colored = ITEM_MOUSEOVER_NONE;
    }


    if(m_item_mouseover != ITEM_MOUSEOVER_NONE && m_item_mouseover != temp){
        GLfloat color_buffer_subdata[24];

        if(m_item_mouseover & 1){
            color = ITEM_COLOR_1 + 0.02;
        }
        else{
            color = ITEM_COLOR_2 + 0.02;
        }

        color_buffer_subdata[0] = color;
        color_buffer_subdata[1] = color;
        color_buffer_subdata[2] = color;
        color_buffer_subdata[3] = 1.0f;
        color_buffer_subdata[4] = color;
        color_buffer_subdata[5] = color;
        color_buffer_subdata[6] = color;
        color_buffer_subdata[7] = 1.0f;
        color_buffer_subdata[8] = color;
        color_buffer_subdata[9] = color;
        color_buffer_subdata[10] = color;
        color_buffer_subdata[11] = 1.0f;
        color_buffer_subdata[12] = color;
        color_buffer_subdata[13] = color;
        color_buffer_subdata[14] = color;
        color_buffer_subdata[15] = 1.0f;
        color_buffer_subdata[16] = color;
        color_buffer_subdata[17] = color;
        color_buffer_subdata[18] = color;
        color_buffer_subdata[19] = 1.0f;
        color_buffer_subdata[20] = color;
        color_buffer_subdata[21] = color;
        color_buffer_subdata[22] = color;
        color_buffer_subdata[23] = 1.0f;

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo_clr);
        glBufferSubData(GL_ARRAY_BUFFER, m_item_mouseover * 24 * sizeof(GLfloat), 24 * sizeof(GLfloat), color_buffer_subdata);
        m_last_item_colored = m_item_mouseover;
    }
}


const std::unique_ptr<BasePart>* PartsPanelGUI::getPickedObject() const{
    return m_picked_part;
}

