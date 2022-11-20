#include <iostream>
#include <cassert>

#include <stb/stb_image.h>

#include "Sprite.hpp"
#include "../core/log.hpp"
#include "../core/RenderContext.hpp"


Sprite::Sprite(const RenderContext* render_context, const math::vec2& pos, short positioning,
               const char* path, float size) : m_pos(pos){
    assert(path);

    m_positioning = positioning;
    m_size = size;
    m_render_context = render_context;
    m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);

    initgl(path);
}

        
Sprite::~Sprite(){

}


void Sprite::initgl(const char* path){
    assert(path);

    int x, y, n;
    unsigned char* image_data = stbi_load(path, &x, &y, &n, 4);
    if(!image_data) {
        std::cerr << "Sprite::Sprite - could not load sprite " << path << std::endl;
        log("Sprite::Sprite - could not load sprite ", path);
    }
    else{
        glGenTextures(1, &m_sprite);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_sprite);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    stbi_image_free(image_data);

    glGenVertexArrays(1, &m_vao);
    m_render_context->bindVao(m_vao);

    m_disp_location = m_render_context->getUniformLocation(SHADER_SPRITE, "disp");

    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glVertexAttribPointer(0, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    updateVertexArray();


    glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glVertexAttribPointer(1, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    GLfloat tex_coords[2 * 6] = {0.0f, 0.0f,
                                 0.0f, 1.0f,
                                 1.0f, 0.0f,
                                 1.0f, 0.0f,
                                 0.0f, 1.0f,
                                 1.0f, 1.0f};
    glBufferData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(GLfloat), tex_coords, GL_STATIC_DRAW);

}


void Sprite::updateVertexArray(){
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    float pos_x = m_positioning == SPRITE_DRAW_ABSOLUTE ? m_pos.v[0] : m_pos.v[0] * m_fb_width;
    float pos_y = m_positioning == SPRITE_DRAW_ABSOLUTE ? m_pos.v[1] : m_pos.v[1] * m_fb_height;
    GLfloat vertices[2 * 6] = {pos_x - m_size / 2, pos_y + m_size / 2,
                               pos_x - m_size / 2, pos_y - m_size / 2,
                               pos_x + m_size / 2, pos_y + m_size / 2,
                               pos_x + m_size / 2, pos_y + m_size / 2,
                               pos_x - m_size / 2, pos_y - m_size / 2,
                               pos_x + m_size / 2, pos_y - m_size / 2};
    glBufferData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
}


void Sprite::updatePos(const math::vec2& pos){
    m_pos = pos;
}


void Sprite::updateSize(const float size){
    m_size = size;
}


void Sprite::onFramebufferSizeUpdate(){
    m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
    if(m_positioning == SPRITE_DRAW_RELATIVE){
        updateVertexArray();
    }
}


void Sprite::render(){
    m_render_context->useProgram(SHADER_SPRITE);
    float disp[] = {0.0f, 0.0f};
    glUniform2fv(m_disp_location, 1.0f, disp);

    m_render_context->bindVao(m_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_sprite);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
