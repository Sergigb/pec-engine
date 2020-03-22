#include "DebugOverlay.hpp"


DebugOverlay::DebugOverlay(int fb_width, int fb_height, GLuint shader){
    m_text_fps = new Text2D(fb_width, fb_height, color{1.0, 1.0, 0.0}, 256, "../data/fonts/Liberastika-Regular.ttf", 15, shader);
    m_text_rendered_objects = new Text2D(fb_width, fb_height, color{1.0, 1.0, 0.0}, 256, "../data/fonts/Liberastika-Regular.ttf", 15, shader);
    debug_info_box(&m_text_debug, fb_width, fb_height, shader);

    m_rendered_obj = 0;
}


DebugOverlay::~DebugOverlay(){
    delete m_text_debug;
    delete m_text_fps;
    delete m_text_rendered_objects;
}


void DebugOverlay::setRenderedObjects(int n){
    m_rendered_obj = n;
}


void DebugOverlay::onFramebufferSizeUpdate(int fb_width, int fb_height){
    m_text_fps->onFramebufferSizeUpdate(fb_width, fb_height);
    m_text_debug->onFramebufferSizeUpdate(fb_width, fb_height);
    m_text_rendered_objects->onFramebufferSizeUpdate(fb_width, fb_height);
}


void DebugOverlay::render(){
    wchar_t buffer[64];
    std::ostringstream oss2;
    
    glDisable(GL_DEPTH_TEST);
    oss2 << (int)get_fps() << " FPS";
    mbstowcs(buffer, oss2.str().c_str(), 64);
    m_text_fps->clearStrings();
    m_text_fps->addString(buffer, 75, 25, 1, STRING_DRAW_ABSOLUTE_TR);
    m_text_fps->render();

    m_text_debug->render();

    oss2.str("");
    oss2.clear();
    m_text_rendered_objects->clearStrings();
    oss2 << "Num rendered objects: " << m_rendered_obj;
    mbstowcs(buffer, oss2.str().c_str(), 64);
    m_text_rendered_objects->addString(buffer, 15, 15, 1, STRING_DRAW_ABSOLUTE_BL);
    m_text_rendered_objects->render();
    glEnable(GL_DEPTH_TEST);

}