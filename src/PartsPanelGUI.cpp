#include "PartsPanelGUI.hpp"


PartsPanelGUI::PartsPanelGUI(float fb_width, float fb_height){
    m_fb_width = fb_width;
    m_fb_height = fb_height;
    m_master_parts_list = nullptr;

    glGenFramebuffers(1, &m_fb);
    glGenTextures(1, &m_tex);

    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb_width, fb_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex, 0);

    GLenum draw_bufs[] = {GL_COLOR_ATTACHMENT0};
    glDrawBuffers(1, draw_bufs);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


PartsPanelGUI::~PartsPanelGUI(){
    // buffers need to be created, we only clear color for now
    //glDeleteBuffers(1, &m_vbo_vert);
    //glDeleteBuffers(1, &m_vbo_tex);
    glDeleteTextures(1, &m_tex);
}


void PartsPanelGUI::setMasterPartList(const std::map<int, std::unique_ptr<BasePart>>* master_parts_list){
    m_master_parts_list = master_parts_list;
}


void PartsPanelGUI::render(){
    glBindFramebuffer(GL_FRAMEBUFFER, m_fb);

    // clear the buffers for now as a test
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we don't have depth?
    glClearColor(1.0, 0.0, 1.0, 0.0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void PartsPanelGUI::onFramebufferSizeUpdate(float fb_width, float fb_height){
    m_fb_width = fb_width;
    m_fb_height = fb_height;

    // glDeleteTextures(1, &m_tex); // not sure if this is needed
    glBindTexture(GL_TEXTURE_2D, m_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, fb_width, fb_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // this is not needed because it's the same handle? test.
    // glBindFramebuffer(GL_FRAMEBUFFER, m_fb);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_tex, 0);
    // GLenum draw_bufs[] = {GL_COLOR_ATTACHMENT0};
    // glDrawBuffers(1, draw_bufs);
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void PartsPanelGUI::bindTexture(){
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_tex);
}


void PartsPanelGUI::update(){
    // deal with the input
}

