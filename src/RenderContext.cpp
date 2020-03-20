#include "RenderContext.hpp"

RenderContext::RenderContext(const Camera* camera, const WindowHandler* window_handler){
    m_camera = camera;
    m_window_handler = window_handler;

    // shader setup

    m_debug_overlay = new DebugOverlay(m_camera, m_window_handler);

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
}

void RenderContext::render(){
    int num_rendered = 0;

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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(m_bg_r, m_bg_g, m_bg_b, m_bg_a);

    m_debug_overlay->setRenderedObjects(num_rendered);
    m_debug_overlay->render();

    // render the stuff    
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

