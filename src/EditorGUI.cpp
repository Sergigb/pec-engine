#include "EditorGUI.hpp"



EditorGUI::EditorGUI(){
    m_fb_update = true;
    m_init = false;
}


EditorGUI::EditorGUI(const WindowHandler* window_handler, FontAtlas* atlas, GLuint shader, const RenderContext* render_context): BaseGUI(window_handler){
    m_fb_update = true;
    m_init = true;
    m_font_atlas = atlas;
    m_shader_programme = shader;
    m_render_context = render_context;

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
    float gui_color[4 * EDITOR_GUI_VERTEX_NUM] = {0.4, 0.4, 0.4, 1.0,
                                                  0.4, 0.4, 0.4, 1.0,
                                                  0.4, 0.4, 0.4, 1.0,
                                                  0.4, 0.4, 0.4, 1.0,
                                                  0.4, 0.4, 0.4, 1.0,
                                                  0.4, 0.4, 0.4, 1.0,
                                                  0.4, 0.4, 0.4, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0,
                                                  0.25, 0.25, 0.25, 1.0};

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

    for(char i=0; i < 3; i++){
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
    float fb_height_f, fb_width_f, x_start;

    GLfloat vertex_buffer[2 * EDITOR_GUI_VERTEX_NUM];

    m_window_handler->getFramebufferSize(fb_width, fb_height);
    fb_width_f = (float)fb_width;
    fb_height_f= (float)fb_height;

    // base panels
    vertex_buffer[0] = 0.0;
    vertex_buffer[1] = 0.0;
    vertex_buffer[2] = 0.0;
    vertex_buffer[3] = fb_height_f;
    vertex_buffer[4] = EDITOR_GUI_LP_W;
    vertex_buffer[5] = 0.0;
    vertex_buffer[6] = EDITOR_GUI_LP_W;
    vertex_buffer[7] = fb_height_f;
    vertex_buffer[8] = 0.0;
    vertex_buffer[9] = fb_height_f - EDITOR_GUI_TP_H;
    vertex_buffer[10] = fb_width_f;
    vertex_buffer[11] = fb_height_f - EDITOR_GUI_TP_H;
    vertex_buffer[12] = fb_width_f;
    vertex_buffer[13] = fb_height_f;
    // buttons

    for(char i=0; i < 3; i++){
        x_start = (i+1) * BUTTON_PAD_X + i * BUTTON_SIZE_X;

        vertex_buffer[14 + i * 8] = x_start; //1
        vertex_buffer[15 + i * 8] = fb_height_f - BUTTON_PAD_Y;
        vertex_buffer[16 + i * 8] = x_start;
        vertex_buffer[17 + i * 8] = fb_height_f - BUTTON_PAD_Y - BUTTON_SIZE_Y;
        vertex_buffer[18 + i * 8] = x_start + BUTTON_SIZE_X;
        vertex_buffer[19 + i * 8] = fb_height_f - BUTTON_PAD_Y - BUTTON_SIZE_Y;
        vertex_buffer[20 + i * 8] = x_start + BUTTON_SIZE_X;
        vertex_buffer[21 + i * 8] = fb_height_f - BUTTON_PAD_Y;
    }

    m_render_context->bindVao(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 2 * EDITOR_GUI_VERTEX_NUM * sizeof(GLfloat), vertex_buffer, GL_STATIC_DRAW);

    /*glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, 2 * m_num_vertices * sizeof(GLfloat), tex_coords_buffer.get(), GL_STATIC_DRAW);*/
}


void EditorGUI::render(){
    if(m_fb_update){
        updateBuffers();
        m_fb_update = false;
    }

    m_render_context->useProgram(m_shader_programme);
    m_render_context->bindVao(m_vao);

    //m_font_atlas->bindTexture();
    glDrawElements(GL_TRIANGLES, EDITOR_GUI_INDEX_NUM, GL_UNSIGNED_SHORT, NULL);
}


void EditorGUI::update(){
    // deal with input??
}

