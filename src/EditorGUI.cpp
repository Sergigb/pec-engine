#include "EditorGUI.hpp"



EditorGUI::EditorGUI(){
    m_fb_update = true;
    m_init = false;
}


EditorGUI::EditorGUI(const FontAtlas* atlas, const RenderContext* render_context, const Input* input){
    m_render_context = render_context;
    m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
    m_fb_update = true;
    m_init = true;
    m_font_atlas = atlas;
    m_gui_shader = m_render_context->getShader(SHADER_GUI);
    m_input = input;
    m_button_mouseover = -1;
    m_last_button_color = -1;
    m_button_select = -1;
    std::memset(m_button_status, 0, EDITOR_GUI_N_BUTTONS * sizeof(bool));
    std::memset(m_button_color_status, 0, EDITOR_GUI_N_BUTTONS * sizeof(bool));

    color c{0.85, 0.85, 0.85};
    m_text_debug.reset(new Text2D(m_fb_width, m_fb_height, c, m_font_atlas, render_context));

    m_parts_panel.reset(new PartsPanelGUI(EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN * 2, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN * 2,
                        m_font_atlas, m_render_context, m_input));

    m_disp_location = glGetUniformLocation(m_gui_shader, "disp");

    // gl init
    glGenVertexArrays(1, &m_vao);
    m_render_context->bindVao(m_vao);

    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glVertexAttribPointer(0, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_vbo_clr);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_clr);
    glVertexAttribPointer(1, 4,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    // color never changes so there's no reason to change it in the updateBuffers method
    GLfloat gui_color[4 * EDITOR_GUI_VERTEX_NUM] = {EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    EDITOR_GUI_PANEL_COLOR,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT,
                                                    BUTTON_COLOR_DEFAULT};

    glBufferData(GL_ARRAY_BUFFER, 4 * EDITOR_GUI_VERTEX_NUM * sizeof(GLfloat), gui_color, GL_STATIC_DRAW);

    glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glVertexAttribPointer(2, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);
    // this works, but the alpha is set all to 0s because I don't have a good texture atlas yet :(
    GLfloat tex_coords[4 * EDITOR_GUI_VERTEX_NUM] = {0.0, 1.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     1.0, 1.0, 0.0,
                                                     1.0, 0.0, 0.0,
                                                     0.0, 1.0, 0.0,
                                                     1.0, 1.0, 0.0,
                                                     1.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,
                                                     0.0, 0.0, 0.0,};

    glBufferData(GL_ARRAY_BUFFER, 4 * EDITOR_GUI_VERTEX_NUM * sizeof(GLfloat), tex_coords, GL_STATIC_DRAW);

    glGenBuffers(1, &m_vbo_ind);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glVertexAttribPointer(3, 3, GL_UNSIGNED_SHORT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(3);

    // same thing for the index buffer
    GLushort index_buffer[EDITOR_GUI_INDEX_NUM];
    index_buffer[0] = 0;
    index_buffer[1] = 2;
    index_buffer[2] = 1;
    index_buffer[3] = 1;
    index_buffer[4] = 2;
    index_buffer[5] = 3;
    index_buffer[6] = 1;
    index_buffer[7] = 4;
    index_buffer[8] = 5;
    index_buffer[9] = 1;
    index_buffer[10] = 5;
    index_buffer[11] = 6;

    // buttons if I'm not mistaken
    for(char i=0; i < EDITOR_GUI_N_BUTTONS; i++){
        int disp = i * 4;

        index_buffer[12 + i * 6] = disp + 7;
        index_buffer[13 + i * 6] = disp + 8;
        index_buffer[14 + i * 6] = disp + 9;
        index_buffer[15 + i * 6] = disp + 9;
        index_buffer[16 + i * 6] = disp + 10;
        index_buffer[17 + i * 6] = disp + 7;
    }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, EDITOR_GUI_INDEX_NUM * sizeof(GLushort), index_buffer, GL_STATIC_DRAW);

    // texture atlas loading test
    int x, y, n;
    unsigned char* image_data = stbi_load("../data/test_texture.png", &x, &y, &n, 4);
    if(!image_data) {
        std::cerr << "EditorGUI::EditorGUI - could not load GUI texture atlas" << std::endl;
        log("EditorGUI::EditorGUI - could not load GUI texture atlas");
    }
    else{
        glGenTextures(1, &m_texture_atlas);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_texture_atlas);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    // part list panel
    glGenVertexArrays(1, &m_parts_panel_vao);
    m_render_context->bindVao(m_parts_panel_vao);

    glGenBuffers(1, &m_parts_panel_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_parts_panel_vbo_vert);
    glVertexAttribPointer(0, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    GLfloat parts_panel_vert[12] = {EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN,
                                    EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN,
                                    EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN};
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), parts_panel_vert, GL_STATIC_DRAW);

    glGenBuffers(1, &m_parts_panel_vbo_clr);
    glBindBuffer(GL_ARRAY_BUFFER, m_parts_panel_vbo_clr);
    glVertexAttribPointer(1, 4,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    GLfloat parts_panel_clr[24] = {0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0};
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), parts_panel_clr, GL_STATIC_DRAW);

    glGenBuffers(1, &m_parts_panel_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_parts_panel_vbo_tex);
    glVertexAttribPointer(2, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);
    GLfloat parts_panel_tex[18] = {0.0, 0.0, 1.0,
                                   1.0, 0.0, 1.0,
                                   0.0, 1.0, 1.0,
                                   1.0, 0.0, 1.0,
                                   1.0, 1.0, 1.0,
                                   0.0, 1.0, 1.0};
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), parts_panel_tex, GL_STATIC_DRAW);
}


EditorGUI::~EditorGUI(){
    if(m_init){
        glDeleteBuffers(1, &m_vbo_vert);
        glDeleteBuffers(1, &m_vbo_tex);
        glDeleteBuffers(1, &m_vbo_ind);
        glDeleteBuffers(1, &m_vbo_clr);
        glDeleteVertexArrays(1, &m_vao);
        glDeleteTextures(1, &m_texture_atlas);

        glDeleteBuffers(1, &m_parts_panel_vbo_vert);
        glDeleteBuffers(1, &m_parts_panel_vbo_tex);
        glDeleteBuffers(1, &m_parts_panel_vbo_clr);
        glDeleteVertexArrays(1, &m_parts_panel_vao);
    }
}


void EditorGUI::onFramebufferSizeUpdate(){
    m_fb_update = true;
}


void EditorGUI::updateBuffers(){
    float x_start;

    GLfloat vertex_buffer[2 * EDITOR_GUI_VERTEX_NUM];

    // left panel
    vertex_buffer[0] = 0.0;
    vertex_buffer[1] = 0.0;
    vertex_buffer[2] = 0.0;
    vertex_buffer[3] = m_fb_height;
    vertex_buffer[4] = EDITOR_GUI_LP_W;
    vertex_buffer[5] = 0.0;
    vertex_buffer[6] = EDITOR_GUI_LP_W;
    // top panel
    vertex_buffer[7] = m_fb_height;
    vertex_buffer[8] = 0.0;
    vertex_buffer[9] = m_fb_height - EDITOR_GUI_TP_H;
    vertex_buffer[10] = m_fb_width;
    vertex_buffer[11] = m_fb_height - EDITOR_GUI_TP_H;
    vertex_buffer[12] = m_fb_width;
    vertex_buffer[13] = m_fb_height;

    // buttons
    m_text_debug->clearStrings();
    for(char i=0; i < EDITOR_GUI_N_BUTTONS; i++){
        x_start = (i+1) * BUTTON_PAD_X + i * BUTTON_SIZE_X;

        m_text_debug->addString(L"Button", x_start + BUTTON_SIZE_X / 2, m_fb_height - BUTTON_PAD_Y - BUTTON_SIZE_Y / 2 + 5,
                                1, STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);

        vertex_buffer[14 + i * 8] = x_start; //1
        vertex_buffer[15 + i * 8] = m_fb_height - BUTTON_PAD_Y;
        vertex_buffer[16 + i * 8] = x_start;
        vertex_buffer[17 + i * 8] = m_fb_height - BUTTON_PAD_Y - BUTTON_SIZE_Y;
        vertex_buffer[18 + i * 8] = x_start + BUTTON_SIZE_X;
        vertex_buffer[19 + i * 8] = m_fb_height - BUTTON_PAD_Y - BUTTON_SIZE_Y;
        vertex_buffer[20 + i * 8] = x_start + BUTTON_SIZE_X;
        vertex_buffer[21 + i * 8] = m_fb_height - BUTTON_PAD_Y;
    }

    m_render_context->bindVao(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 2 * EDITOR_GUI_VERTEX_NUM * sizeof(GLfloat), vertex_buffer, GL_STATIC_DRAW);

    // parts panel
    GLfloat parts_panel_vert[12] = {EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN,
                                    EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, EDITOR_GUI_PP_MARGIN,
                                    EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN,
                                    EDITOR_GUI_PP_MARGIN, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN};
    glBindBuffer(GL_ARRAY_BUFFER, m_parts_panel_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), parts_panel_vert, GL_STATIC_DRAW);
}

void EditorGUI::colorButton(const GLfloat* color_array, int button){
        m_render_context->bindVao(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo_clr);

        glBufferSubData(GL_ARRAY_BUFFER, ((7*4) + (button * 16)) * sizeof(GLfloat) , 16 * sizeof(GLfloat), color_array);
}


void EditorGUI::updateButtons(){ // used to update button colors
    // grab the current value of m_button_mouseover in case it is overwritten in the update function while we are working with it
    int button_mouseover = m_button_mouseover;

    // reset button color if m_last_button_color != -1
    if(m_last_button_color >= 0){
        if(!m_button_status[m_last_button_color]){
            GLfloat new_color[16] = {BUTTON_COLOR_DEFAULT,
                                     BUTTON_COLOR_DEFAULT,
                                     BUTTON_COLOR_DEFAULT,
                                     BUTTON_COLOR_DEFAULT};

            colorButton(new_color, m_last_button_color);
        }
    }
    m_last_button_color = -1;

    if(button_mouseover >= 0 && button_mouseover < EDITOR_GUI_N_BUTTONS){ // && button_mouseover < EDITOR_GUI_N_BUTTONS -> sanity check
        if(m_button_status[button_mouseover]){
            GLfloat new_color[16] = {BUTTON_COLOR_SELECTED_MOUSEOVER,
                                     BUTTON_COLOR_SELECTED_MOUSEOVER,
                                     BUTTON_COLOR_SELECTED_MOUSEOVER,
                                     BUTTON_COLOR_SELECTED_MOUSEOVER};
            colorButton(new_color, button_mouseover);
            m_button_color_status[button_mouseover] = false;
        }
        else{
            GLfloat new_color[16] = {BUTTON_COLOR_MOUSEOVER,
                                     BUTTON_COLOR_MOUSEOVER,
                                     BUTTON_COLOR_MOUSEOVER,
                                     BUTTON_COLOR_MOUSEOVER};
            colorButton(new_color, button_mouseover);
        }

        m_last_button_color = button_mouseover;

        return;
    }

    for(int i=0; i < EDITOR_GUI_N_BUTTONS; i++){
        if(m_button_color_status[i] != m_button_status[i]){
            m_button_color_status[i] = m_button_status[i];

            if(m_button_color_status[i]){
                GLfloat new_color[16] = {BUTTON_COLOR_SELECTED,
                                         BUTTON_COLOR_SELECTED,
                                         BUTTON_COLOR_SELECTED,
                                         BUTTON_COLOR_SELECTED};

                colorButton(new_color, i);
            }
            else{
                GLfloat new_color[16] = {BUTTON_COLOR_DEFAULT,
                                         BUTTON_COLOR_DEFAULT,
                                         BUTTON_COLOR_DEFAULT,
                                         BUTTON_COLOR_DEFAULT};
                colorButton(new_color, i);
            }
        }
    }
}


void EditorGUI::render(){
    if(m_fb_update){
        m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
        m_text_debug->onFramebufferSizeUpdate(m_fb_width, m_fb_height);
        m_parts_panel->onFramebufferSizeUpdate(EDITOR_GUI_LP_W - EDITOR_GUI_PP_MARGIN * 2, m_fb_height - EDITOR_GUI_TP_H - EDITOR_GUI_PP_MARGIN * 2);

        updateBuffers();
        m_fb_update = false;
    }
    updateButtons();

    m_render_context->useProgram(m_gui_shader);
    glUniform2f(m_disp_location, 0.0, 0.0);

    m_render_context->bindVao(m_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texture_atlas);
    glDrawElements(GL_TRIANGLES, EDITOR_GUI_INDEX_NUM, GL_UNSIGNED_SHORT, NULL);

    m_parts_panel->render();
    m_render_context->useProgram(m_gui_shader);
    m_parts_panel->bindTexture();
    m_render_context->bindVao(m_parts_panel_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_text_debug->render();
}


int EditorGUI::update(){
    double posx, posy;
    int lp_action;

    m_input->getMousePos(posx, posy);
    posx = (double)posx;
    posy = m_fb_height - (double)posy;

    m_button_mouseover = -1;
    if(posy > m_fb_height - EDITOR_GUI_TP_H){ // mouse over top panel
        for(int i=0; i < EDITOR_GUI_N_BUTTONS; i++){
            float x_start = (i+1) * BUTTON_PAD_X + i * BUTTON_SIZE_X;
            
            if(posx > x_start && posx < x_start + BUTTON_SIZE_X && posy > m_fb_height - 
               BUTTON_PAD_Y - BUTTON_SIZE_Y && posy < m_fb_height - BUTTON_PAD_Y){

                if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS){
                    m_button_status[i] = !m_button_status[i];
                }
                m_button_mouseover = i;
            }
        }
    }

    lp_action = m_parts_panel->update(posx - EDITOR_GUI_PP_MARGIN, posy - EDITOR_GUI_PP_MARGIN); // transform coord origin

    // may change
    if(lp_action){
        return EDITOR_ACTION_OBJECT_PICK;
    }
    else{
        // focus check
        if(posy > m_fb_height - EDITOR_GUI_TP_H || posx < EDITOR_GUI_LP_W){
            return EDITOR_ACTION_FOCUS;
        }
        else{
            return EDITOR_ACTION_NONE;
        }
    }
}


void EditorGUI::setMasterPartList(const std::map<std::uint32_t, std::unique_ptr<BasePart>>* master_parts_list){
    m_parts_panel->setMasterPartList(master_parts_list);
}


const std::unique_ptr<BasePart>* EditorGUI::getPickedObject() const{
    return m_parts_panel->getPickedObject();
}

