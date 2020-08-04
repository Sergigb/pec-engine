#include "PartsPanelGUI.hpp"


PartsPanelGUI::PartsPanelGUI(float fb_width, float fb_height, const FontAtlas* atlas, const RenderContext* render_context){
    m_fb_width = fb_width;
    m_fb_height = fb_height;
    m_master_parts_list = nullptr;
    m_font_atlas = atlas;
    m_render_context = render_context;
    //color c{0.4, 0.9, 0.9};
  //  m_text.reset(new Text2D(fb_width, fb_height, c, m_font_atlas, text_shader, render_context));

   // m_text->addString(L"Test string\nthis is just a test\ningore this, thank you", 50, 50,
     //                           1, STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_LEFT);

    m_gui_shader = m_render_context->getShader(SHADER_GUI);
    m_projection_location = glGetUniformLocation(m_gui_shader, "projection");
    m_projection = math::orthographic(fb_width, 0, fb_height, 0, 1.0f , -1.0f);

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
    GLfloat vert[12] = {10.0, 10.0,
                        100.0, 10.0,
                        10.0, 100.0,
                        100.0, 10.0,
                        100.0, 100.0,
                        10.0, 100.0};
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), vert, GL_STATIC_DRAW);

    glGenBuffers(1, &m_vbo_clr);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_clr);
    glVertexAttribPointer(1, 4,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    GLfloat color[24] = {0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0,
                         0.0, 0.0, 0.0, 0.0};
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), color, GL_STATIC_DRAW);

    glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glVertexAttribPointer(2, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);
    GLfloat texture[18] = {0.0, 1.0, 1.0,
                           1.0, 1.0, 1.0,
                           0.0, 0.0, 1.0,
                           1.0, 1.0, 1.0,
                           1.0, 0.0, 1.0,
                           0.0, 0.0, 1.0};
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), texture, GL_STATIC_DRAW);
}


PartsPanelGUI::~PartsPanelGUI(){
    // buffers need to be created, we only clear color for now
    //glDeleteBuffers(1, &m_vbo_vert);
    //glDeleteBuffers(1, &m_vbo_tex);
    glDeleteTextures(1, &m_tex);
    glDeleteFramebuffers(1, &m_fb);
}


void PartsPanelGUI::setMasterPartList(const std::map<int, std::unique_ptr<BasePart>>* master_parts_list){
    m_master_parts_list = master_parts_list;
}


void PartsPanelGUI::render(){
    glBindFramebuffer(GL_FRAMEBUFFER, m_fb);

    glClearColor(0.25, 0.25, 0.25, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_render_context->useProgram(m_gui_shader);
    glUniformMatrix4fv(m_projection_location, 1, GL_FALSE, m_projection.m);
    glViewport(0, 0, m_fb_width, m_fb_height);

    m_render_context->bindVao(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
   // m_text->render();*/

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // restore the original framebuffer uniform
    float fb_width, fb_height;
    m_render_context->getDefaultFbSize(fb_width, fb_height);
    m_render_context->useProgram(m_gui_shader);
    math::mat4 projection = math::orthographic(fb_width, 0, fb_height, 0, 1.0f , -1.0f); // we have to compute that each frame, not ideal
    glUniformMatrix4fv(m_projection_location, 1, GL_FALSE, projection.m);
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
}


void PartsPanelGUI::bindTexture(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex);
}


void PartsPanelGUI::update(){
    // deal with the input
}

