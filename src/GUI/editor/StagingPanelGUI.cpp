//#include <cstdlib>
//#include <cstring>

#include "StagingPanelGUI.hpp"
#include "../FontAtlas.hpp"
#include "../Text2D.hpp"
#include "../../core/RenderContext.hpp"
#include "../../core/Input.hpp"


StagingPanelGUI::StagingPanelGUI(float fb_width, float fb_height, const FontAtlas* atlas, const RenderContext* render_context, const Input* input){
    m_fb_width = fb_width;
    m_fb_height = fb_height;
    m_font_atlas = atlas;
    m_render_context = render_context;
    m_input = input;

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

    m_text->addString(L"Test string", 10., 100., 1, STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);
    m_text->addString(L"Hello", 10., 150., 1, STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);
    m_text->addString(L"fffusfksj", 10., 200., 1, STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);

    /*GLenum draw_bufs[] = {GL_COLOR_ATTACHMENT0};
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
    glEnableVertexAttribArray(2);*/
}


StagingPanelGUI::~StagingPanelGUI(){
    //glDeleteBuffers(1, &m_vbo_vert);
    //glDeleteBuffers(1, &m_vbo_tex);
    //glDeleteBuffers(1, &m_vbo_clr);
    //glDeleteVertexArrays(1, &m_vao);
    glDeleteTextures(1, &m_tex);
    glDeleteFramebuffers(1, &m_fb);
}


void StagingPanelGUI::render(){
    float fb_width, fb_height;
    math::mat4 projection;

    glBindFramebuffer(GL_FRAMEBUFFER, m_fb);
    glViewport(0, 0, m_fb_width, m_fb_height);
    glClearColor(0.25, 0.25, 0.25, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /*m_render_context->useProgram(SHADER_GUI);
    glUniformMatrix4fv(m_projection_location, 1, GL_FALSE, m_projection.m);
    glUniform2fv(m_disp_location, 1, m_panel_scroll.v);
    m_render_context->bindVao(m_vao);
    buttonMouseoverColor();
    glDrawArrays(GL_TRIANGLES, 0, m_num_vert);*/

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


void StagingPanelGUI::bindTexture(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex);
}


void StagingPanelGUI::onFramebufferSizeUpdate(float fb_width, float fb_height){
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
    //updateBuffers();
}

