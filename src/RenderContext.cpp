#include "RenderContext.hpp"

RenderContext::RenderContext(const Camera* camera, const WindowHandler* window_handler, render_buffers* buff_manager){
    int fb_width, fb_height;
    window_handler->getFramebufferSize(fb_width, fb_height);

    m_camera = camera;
    m_window_handler = window_handler;
    m_buffers = buff_manager;

    initGl();
    log_gl_params();

    m_att_point_scale = math::identity_mat4();
    m_att_point_scale.m[0] = 0.25;
    m_att_point_scale.m[5] = 0.25;
    m_att_point_scale.m[10] = 0.25;

    m_bound_vao = 0;
    m_bound_programme = 0;

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
    m_debug_overlay.reset(new DebugOverlay(fb_width, fb_height, m_text_shader, this));

    // other gl stuff
    m_bg_r = 0.2;
    m_bg_g = 0.2;
    m_bg_b = 0.2;
    m_bg_a = 1.;

    glClearColor(m_bg_r, m_bg_g, m_bg_b, m_bg_a);
}


RenderContext::~RenderContext(){
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


void RenderContext::render(bool render_asynch){
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

    if(render_asynch || m_buffers->last_updated == none){
        for(uint i=0; i<m_objects->size(); i++){
            num_rendered += m_objects->at(i)->render();
        }
        for(uint i=0; i<m_parts->size(); i++){
            const std::vector<struct attachment_point>* att_points = m_parts->at(i)->getAttachmentPoints();

            if(att_points->size()){
                math::mat4 body_transform, att_transform;
                btVector3 point;

                body_transform = m_parts->at(i)->getRigidBodyTransformSingle();
                point = m_parts->at(i)->getParentAttachmentPoint()->point;
                att_transform = body_transform * math::translate(math::identity_mat4(), math::vec3(point.getX(), point.getY(), point.getZ()));
                m_att_point_model->setMeshColor(math::vec3(0.0, 1.0, 0.0));
                num_rendered += m_att_point_model->render(att_transform * m_att_point_scale);

                for(uint j=0; j<att_points->size(); j++){
                    point = att_points->at(j).point;
                    att_transform = body_transform * math::translate(math::identity_mat4(), math::vec3(point.getX(), point.getY(), point.getZ()));
                    m_att_point_model->setMeshColor(math::vec3(1.0, 0.0, 0.0));
                    num_rendered += m_att_point_model->render(att_transform * m_att_point_scale);
                }
            }

            num_rendered += m_parts->at(i)->render();
        }
    }
    else{
        if(m_buffers->last_updated == buffer_1){
            m_buffers->buffer1_lock.lock(); // extremely unlikely to not get the lock
            for(uint i=0; i<m_buffers->buffer1.size(); i++){
                num_rendered += m_buffers->buffer1.at(i).object_ptr->render(m_buffers->buffer1.at(i).transform);
            }
            m_buffers->buffer1_lock.unlock();
        }
        else{
            m_buffers->buffer2_lock.lock();
            for(uint i=0; i<m_buffers->buffer2.size(); i++){
                num_rendered += m_buffers->buffer2.at(i).object_ptr->render(m_buffers->buffer2.at(i).transform);
            }
            m_buffers->buffer2_lock.unlock();
        }
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


void RenderContext::setObjectVector(std::vector<std::unique_ptr<Object>>* objects){
    m_objects = objects;
}


void RenderContext::setPartVector(std::vector<std::unique_ptr<BasePart>>* parts){
    m_parts = parts;
}


void RenderContext::setDebugOverlayPhysicsTimes(double physics_load_time, double physics_sleep_time){
    m_debug_overlay->setPhysicsTimes(physics_load_time, physics_sleep_time);
}


void RenderContext::setAttPointModel(std::unique_ptr<Model>* att_point_model){
    m_att_point_model = std::move(*att_point_model);
}


GLuint RenderContext::getBoundShader() const{
    return m_bound_vao;
}


GLuint RenderContext::getBoundVao() const{
    return m_bound_programme;
}


void RenderContext::useProgram(GLuint program) const{
    if(program != m_bound_programme){
        glUseProgram(program);
    }
}


void RenderContext::bindVao(GLuint vao) const{
    if(vao != m_bound_vao){
        glBindVertexArray(vao);
    }
}

