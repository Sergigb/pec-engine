#include "EditorGUI.hpp"



EditorGUI::EditorGUI(){
    m_fb_update = true;
    m_init = false;
}


EditorGUI::EditorGUI(const WindowHandler* window_handler, FontAtlas* atlas, GLuint shader, const RenderContext* render_context, const Input* input): BaseGUI(window_handler){
    int fb_height, fb_width;

    m_fb_update = true;
    m_init = true;
    m_font_atlas = atlas;
    m_shader_programme = shader;
    m_render_context = render_context;
    m_input = input;
    m_button_mouseover = -1;
    m_last_button_color = -1;
    m_button_select = -1;
    std::memset(m_button_status, 0, EDITOR_GUI_N_BUTTONS * sizeof(bool));
    std::memset(m_button_color_status, 0, EDITOR_GUI_N_BUTTONS * sizeof(bool));

    m_window_handler->getFramebufferSize(fb_width, fb_height);
    m_fb_width_f = (float)fb_width;
    m_fb_height_f = (float)fb_height;

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

    /*glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glVertexAttribPointer(1, 2,  GL_FLOAT, GL_FALSE, 0, NULL); // CHANGE THE ATTRIBUTE LOCATION!!!!
    glEnableVertexAttribArray(1);*/ // CHANGE THE ATTRIBUTE LOCATION!!!!

    glGenBuffers(1, &m_vbo_ind);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glVertexAttribPointer(2, 3, GL_UNSIGNED_SHORT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);

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
}


EditorGUI::~EditorGUI(){
    if(m_init){
        glDeleteBuffers(1, &m_vbo_vert);
        //glDeleteBuffers(1, &m_vbo_tex);
        glDeleteBuffers(1, &m_vbo_ind);
        glDeleteVertexArrays(1, &m_vao);
    }
}


void EditorGUI::onFramebufferSizeUpdate(){
    m_fb_update = true;
}


void EditorGUI::updateBuffers(){
    int fb_height, fb_width;
    float x_start;

    GLfloat vertex_buffer[2 * EDITOR_GUI_VERTEX_NUM];

    m_window_handler->getFramebufferSize(fb_width, fb_height);
    m_fb_width_f = (float)fb_width;
    m_fb_height_f = (float)fb_height;

    // left panel
    vertex_buffer[0] = 0.0;
    vertex_buffer[1] = 0.0;
    vertex_buffer[2] = 0.0;
    vertex_buffer[3] = m_fb_height_f;
    vertex_buffer[4] = EDITOR_GUI_LP_W;
    vertex_buffer[5] = 0.0;
    vertex_buffer[6] = EDITOR_GUI_LP_W;
    // top panel
    vertex_buffer[7] = m_fb_height_f;
    vertex_buffer[8] = 0.0;
    vertex_buffer[9] = m_fb_height_f - EDITOR_GUI_TP_H;
    vertex_buffer[10] = m_fb_width_f;
    vertex_buffer[11] = m_fb_height_f - EDITOR_GUI_TP_H;
    vertex_buffer[12] = m_fb_width_f;
    vertex_buffer[13] = m_fb_height_f;

    // buttons
    for(char i=0; i < EDITOR_GUI_N_BUTTONS; i++){
        x_start = (i+1) * BUTTON_PAD_X + i * BUTTON_SIZE_X;

        vertex_buffer[14 + i * 8] = x_start; //1
        vertex_buffer[15 + i * 8] = m_fb_height_f - BUTTON_PAD_Y;
        vertex_buffer[16 + i * 8] = x_start;
        vertex_buffer[17 + i * 8] = m_fb_height_f - BUTTON_PAD_Y - BUTTON_SIZE_Y;
        vertex_buffer[18 + i * 8] = x_start + BUTTON_SIZE_X;
        vertex_buffer[19 + i * 8] = m_fb_height_f - BUTTON_PAD_Y - BUTTON_SIZE_Y;
        vertex_buffer[20 + i * 8] = x_start + BUTTON_SIZE_X;
        vertex_buffer[21 + i * 8] = m_fb_height_f - BUTTON_PAD_Y;
    }

    m_render_context->bindVao(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 2 * EDITOR_GUI_VERTEX_NUM * sizeof(GLfloat), vertex_buffer, GL_STATIC_DRAW);

    /*glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, 2 * m_num_vertices * sizeof(GLfloat), tex_coords_buffer.get(), GL_STATIC_DRAW);*/
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
        GLfloat new_color[16] = {BUTTON_COLOR_DEFAULT,
                                 BUTTON_COLOR_DEFAULT,
                                 BUTTON_COLOR_DEFAULT,
                                 BUTTON_COLOR_DEFAULT};

        colorButton(new_color, m_last_button_color);
    }
    m_last_button_color = -1;

    if(button_mouseover >= 0 && button_mouseover < EDITOR_GUI_N_BUTTONS){ // && button_mouseover < EDITOR_GUI_N_BUTTONS -> sanity check
        if(!m_button_color_status[button_mouseover]){
            GLfloat new_color[16] = {BUTTON_COLOR_MOUSEOVER,
                                     BUTTON_COLOR_MOUSEOVER,
                                     BUTTON_COLOR_MOUSEOVER,
                                     BUTTON_COLOR_MOUSEOVER};

            colorButton(new_color, button_mouseover);

            m_last_button_color = button_mouseover;
        }
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
        updateBuffers();
        m_fb_update = false;
    }
    updateButtons();

    m_render_context->useProgram(m_shader_programme);
    m_render_context->bindVao(m_vao);

    //m_font_atlas->bindTexture();
    glDrawElements(GL_TRIANGLES, EDITOR_GUI_INDEX_NUM, GL_UNSIGNED_SHORT, NULL);
}


void EditorGUI::update(){
    double posx, posy;

    m_input->getMousePos(posx, posy);
    posx = (double)posx;
    posy = m_fb_height_f - (double)posy;

    m_button_mouseover = -1;
    if(posy > m_fb_height_f - EDITOR_GUI_TP_H){ // mouse over top panel
        for(int i=0; i < EDITOR_GUI_N_BUTTONS; i++){
            float x_start = (i+1) * BUTTON_PAD_X + i * BUTTON_SIZE_X;
            
            if(posx > x_start && posx < x_start + BUTTON_SIZE_X && posy > m_fb_height_f - 
               BUTTON_PAD_Y - BUTTON_SIZE_Y && posy < m_fb_height_f - BUTTON_PAD_Y){

                if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS){
                    m_button_status[i] = !m_button_status[i];
                }
                else{
                    m_button_mouseover = i;
                }
            }
        }
    }
}

