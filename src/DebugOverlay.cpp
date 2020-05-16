#include "DebugOverlay.hpp"


DebugOverlay::DebugOverlay(int fb_width, int fb_height, GLuint shader){
    m_text_dynamic_text.reset(new Text2D(fb_width, fb_height, color{1.0, 1.0, 0.0}, 256, "../data/fonts/Liberastika-Regular.ttf", 15, shader));
    Text2D* text_debug;
    debug_info_box(&text_debug, fb_width, fb_height, shader);
    m_text_debug.reset(text_debug);

    m_rendered_obj = 0;
    m_physics_load_time = 0.0;
    m_physics_sleep_time = 0.0;
}


DebugOverlay::~DebugOverlay(){
}


void DebugOverlay::setRenderedObjects(int n){
    m_rendered_obj = n;
}


void DebugOverlay::onFramebufferSizeUpdate(int fb_width, int fb_height){
    m_text_dynamic_text->onFramebufferSizeUpdate(fb_width, fb_height);
    m_text_debug->onFramebufferSizeUpdate(fb_width, fb_height);
}


void DebugOverlay::setPhysicsTimes(double physics_load_time, double physics_sleep_time){
    m_physics_load_time = physics_load_time;
    m_physics_sleep_time = physics_sleep_time;

}


void DebugOverlay::render(){
    wchar_t buffer[64];
    std::ostringstream oss2;
    
    glDisable(GL_DEPTH_TEST);
    oss2 << (int)get_fps() << " FPS";
    mbstowcs(buffer, oss2.str().c_str(), 64);
    m_text_dynamic_text->clearStrings();
    m_text_dynamic_text->addString(buffer, 75, 25, 1, STRING_DRAW_ABSOLUTE_TR);

    oss2.str("");
    oss2.clear();
    oss2 << "Num rendered objects: " << m_rendered_obj;
    mbstowcs(buffer, oss2.str().c_str(), 64);
    m_text_dynamic_text->addString(buffer, 15, 15, 1, STRING_DRAW_ABSOLUTE_BL);

    oss2.str("");
    oss2.clear();
    oss2 << "Physics load: " << std::setprecision(3) << m_physics_load_time << "ms - Sleep time: " << std::setprecision(3) << m_physics_sleep_time << "ms";
    mbstowcs(buffer, oss2.str().c_str(), 64);
    m_text_dynamic_text->addString(buffer, 15, 125, 1, STRING_DRAW_ABSOLUTE_TL);

    m_text_dynamic_text->render();

    m_text_debug->render();

    glEnable(GL_DEPTH_TEST);

}


void debug_info_box(Text2D** t, int fb_width, int fb_height, GLuint shader){
    *t = new Text2D(fb_width, fb_height, color{1.0, 1.0, 0.0}, 256, "../data/fonts/Liberastika-Regular.ttf", 15, shader);
    
    const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
    const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model
    const GLubyte* gl_version = glGetString(GL_VERSION);
    char modelname[64];
    std::ostringstream oss;
    wchar_t vendor_w[128];
    wchar_t renderer_w[128];
    wchar_t glversion_w[128];
    wchar_t modelname_w[64];
    wchar_t totalmemory_w[32];
    unsigned long long mem_bytes;
    float mem_gb;

    wstrcpy(vendor_w, L"Vendor: ", 128);
    wstrcpy(renderer_w, L"Renderer: ", 128);
    wstrcpy(glversion_w, L"GL version: ", 128);

    ucs2wcs(vendor_w+8, vendor, 128-8);
    ucs2wcs(renderer_w+10, renderer, 128-10);
    ucs2wcs(glversion_w+12, gl_version, 128-12);
    
    (*t)->addString(vendor_w, 15, 25, 1, STRING_DRAW_ABSOLUTE_TL);
    (*t)->addString(renderer_w, 15, 45, 1, STRING_DRAW_ABSOLUTE_TL);
    (*t)->addString(glversion_w, 15, 65, 1, STRING_DRAW_ABSOLUTE_TL);

    get_cpu_model(modelname);
    mbstowcs(modelname_w, modelname, 64);
    (*t)->addString(modelname_w+8, 15, 85, 1, STRING_DRAW_ABSOLUTE_TL);

    mem_bytes = get_sys_memory();
    mem_gb = (float)mem_bytes / 0x40000000;
    oss.precision(3);
    oss << "System memory: " << mem_gb << " GB";
    mbstowcs(totalmemory_w, oss.str().c_str(), 64);
    (*t)->addString(totalmemory_w, 15, 105, 1, STRING_DRAW_ABSOLUTE_TL);
}

