#include "RenderContext.hpp"

RenderContext::RenderContext(const Camera* camera, const WindowHandler* window_handler){
    int fb_width, fb_height;
    window_handler->getFramebufferSize(fb_width, fb_height);

    m_camera = camera;
    m_window_handler = window_handler;

    initGl();
    log_gl_params();

    // shader setup

    m_pb_notex_shader = create_programme_from_files("../shaders/phong_blinn_color_vs.glsl",
                                                    "../shaders/phong_blinn_color_fs.glsl");
    log_programme_info(m_pb_notex_shader);
    m_pb_notex_view_mat = glGetUniformLocation(m_pb_notex_shader, "view");
    m_pb_notex_proj_mat = glGetUniformLocation(m_pb_notex_shader, "proj");
    m_pb_notex_light_pos = glGetUniformLocation(m_pb_notex_shader, "light_pos");

    glUseProgram(m_pb_notex_shader);
    glUniformMatrix4fv(m_pb_notex_view_mat, 1, GL_FALSE, m_camera->getViewMatrix().m);
    glUniformMatrix4fv(m_pb_notex_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);
    glUniform3fv(m_pb_notex_light_pos, 1, math::vec3(0.0, 0.0, 0.0).v);

    m_pb_shader = create_programme_from_files("../shaders/phong_blinn_vs.glsl",
                                              "../shaders/phong_blinn_fs.glsl");
    log_programme_info(m_pb_shader);
    m_pb_view_mat = glGetUniformLocation(m_pb_shader, "view");
    m_pb_proj_mat = glGetUniformLocation(m_pb_shader, "proj");
    m_pb_light_pos = glGetUniformLocation(m_pb_shader, "light_pos");

    glUseProgram(m_pb_shader);
    glUniformMatrix4fv(m_pb_view_mat, 1, GL_FALSE, m_camera->getViewMatrix().m);
    glUniformMatrix4fv(m_pb_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);
    glUniform3fv(m_pb_light_pos, 1, math::vec3(0.0, 0.0, 0.0).v);

    m_text_shader = create_programme_from_files("../shaders/text_vs.glsl",
                                                "../shaders/text_fs.glsl");
    m_text_proj_mat = glGetUniformLocation(m_text_shader, "projection");

    math::mat4 text_orto_proj = math::orthographic(fb_width, 0, fb_height , 0, 1.0f , -1.0f);
    glUseProgram(m_text_shader);
    glUniformMatrix4fv(m_text_proj_mat, 1, GL_FALSE, text_orto_proj.m);

    // debug overlay
    m_debug_overlay = new DebugOverlay(m_window_handler, m_text_shader);

    // other gl stuff
    m_bg_r = 0.2;
    m_bg_g = 0.2;
    m_bg_b = 0.2;
    m_bg_a = 1.;

    glClearColor(m_bg_r, m_bg_g, m_bg_b, m_bg_a);
}


RenderContext::~RenderContext(){
    delete m_debug_overlay;
    glDeleteShader(m_pb_notex_shader);
    glDeleteShader(m_pb_shader);
    glDeleteShader(m_text_shader);
}


void RenderContext::initGl(){
    log("Starting GLEW");
    glewExperimental = GL_TRUE;
    glewInit();

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;
    log("Renderer: ", renderer, ", using OpenGL version: ", version);

    // general gl setup
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    //glfwSwapInterval(0);

    int samples;
    glGetIntegerv(GL_SAMPLES, &samples);
    if (samples)
        log("MSAA is available with ", samples, " samples");
    else
        log("MSAA is unavailable");
}


void RenderContext::render(){
    int num_rendered = 0;

    //clean up this shit

    glUseProgram(m_pb_notex_shader);
    if(m_camera->hasMoved())
        glUniformMatrix4fv(m_pb_notex_view_mat, 1, GL_FALSE, m_camera->getViewMatrix().m);
    if(m_camera->projChanged())
        glUniformMatrix4fv(m_pb_notex_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    // I evaluate this again to avoid changing the bound shader too many times, not sure if matters at all
    glUseProgram(m_pb_shader);
    if(m_camera->hasMoved())
        glUniformMatrix4fv(m_pb_view_mat, 1, GL_FALSE, m_camera->getViewMatrix().m);
    if(m_camera->projChanged())
        glUniformMatrix4fv(m_pb_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    if(m_camera->projChanged()){
        int fb_width, fb_height;
        m_window_handler->getFramebufferSize(fb_width, fb_height);

        glUseProgram(m_text_shader);
        math::mat4 projection = math::orthographic(fb_width, 0, fb_height, 0, 1.0f , -1.0f);
        glUniformMatrix4fv(m_text_proj_mat, 1, GL_FALSE, projection.m);

        m_debug_overlay->onFramebufferSizeUpdate(fb_width, fb_height);
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(m_bg_r, m_bg_g, m_bg_b, m_bg_a);

    for(uint i=0; i<m_objects.size(); i++){
        num_rendered += m_objects.at(i)->render();
    }

    m_debug_overlay->setRenderedObjects(num_rendered);
    m_debug_overlay->render();
}


void RenderContext::setLightPosition(const math::vec3& pos) const{
    glUseProgram(m_pb_notex_shader);
    glUniform3fv(m_pb_notex_light_pos, 1, pos.v);
    glUseProgram(m_pb_shader);
    glUniform3fv(m_pb_light_pos, 1, pos.v);
}


GLuint RenderContext::getShader(int shader) const{
    if(shader == SHADER_PHONG_BLINN){
        return m_pb_shader;
    }
    if(shader == SHADER_PHONG_BLINN_NO_TEXTURE){
        return m_pb_notex_shader;
    }
    return 0;
}

