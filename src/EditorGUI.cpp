#include "EditorGUI.hpp"



EditorGUI::EditorGUI(){
    m_fb_update = false;
    m_init = false;
}


EditorGUI::EditorGUI(const WindowHandler* window_handler, FontAtlas* atlas, GLuint shader, const RenderContext* render_context): BaseGUI(window_handler){
    m_fb_update = false;
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

    /*glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glVertexAttribPointer(1, 2,  GL_FLOAT, GL_FALSE, 0, NULL); // CHANGE THE ATTRIBUTE LOCATION!!!!
    glEnableVertexAttribArray(1);*/ // CHANGE THE ATTRIBUTE LOCATION!!!!

    glGenBuffers(1, &m_vbo_ind);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glVertexAttribPointer(1, 3, GL_UNSIGNED_SHORT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
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
    float fb_height_f, fb_width_f;

    std::unique_ptr<GLfloat[]> vertex_buffer;
    //std::unique_ptr<GLfloat[]> tex_coords_buffer; // not using textures yet
    std::unique_ptr<GLushort[]> index_buffer;

    vertex_buffer.reset(new GLfloat[2 * EDITOR_GUI_VERTEX_NUM]);
    index_buffer.reset(new GLushort[EDITOR_GUI_INDEX_NUM]);

    m_window_handler->getFramebufferSize(fb_width, fb_height);
    fb_height_f = (float)fb_width;
    fb_width_f = (float)fb_height;

    // uuuhh...
    vertex_buffer.get()[0] = 0.0;             // 0
    vertex_buffer.get()[1] = 0.0;
    vertex_buffer.get()[2] = 0.0;             // 1
    vertex_buffer.get()[3] = fb_height_f;
    vertex_buffer.get()[4] = EDITOR_GUI_LP_W; // 2
    vertex_buffer.get()[5] = 0.0;
    vertex_buffer.get()[6] = EDITOR_GUI_LP_W; // 3
    vertex_buffer.get()[7] = fb_height_f;
    vertex_buffer.get()[8] = 0.0;             // 4
    vertex_buffer.get()[9] = fb_height_f - EDITOR_GUI_TP_H;
    vertex_buffer.get()[10] = fb_width_f;     // 5
    vertex_buffer.get()[11] = fb_height_f - EDITOR_GUI_TP_H;
    vertex_buffer.get()[12] = fb_width_f;     // 6
    vertex_buffer.get()[13] = fb_height_f;

    index_buffer.get()[0] = 0;
    index_buffer.get()[1] = 1;
    index_buffer.get()[2] = 2;
    index_buffer.get()[3] = 2;
    index_buffer.get()[4] = 1;
    index_buffer.get()[5] = 3;
    index_buffer.get()[6] = 4;
    index_buffer.get()[7] = 1;
    index_buffer.get()[8] = 5;
    index_buffer.get()[9] = 5;
    index_buffer.get()[10] = 1;
    index_buffer.get()[11] = 6;

    m_render_context->bindVao(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 2 * EDITOR_GUI_VERTEX_NUM * sizeof(GLfloat), vertex_buffer.get(), GL_STATIC_DRAW);

    /*glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, 2 * m_num_vertices * sizeof(GLfloat), tex_coords_buffer.get(), GL_STATIC_DRAW);*/

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, EDITOR_GUI_INDEX_NUM * sizeof(GLushort), index_buffer.get(), GL_STATIC_DRAW);
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