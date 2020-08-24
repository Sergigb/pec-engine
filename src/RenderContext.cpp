#include "RenderContext.hpp"

RenderContext::RenderContext(const Camera* camera, const WindowHandler* window_handler, render_buffers* buff_manager){
    int fb_width, fb_height;
    window_handler->getFramebufferSize(fb_width, fb_height);
    m_fb_width = fb_width;
    m_fb_height = fb_height;

    m_camera = camera;
    m_window_handler = window_handler;
    m_buffers = buff_manager;
    m_draw_overlay = false;

    initGl();
    log_gl_params();
    initImgui();

    m_att_point_scale = math::identity_mat4();
    m_att_point_scale.m[0] = 0.25;
    m_att_point_scale.m[5] = 0.25;
    m_att_point_scale.m[10] = 0.25;

    m_bound_vao = 0;
    m_bound_programme = 0;

    m_pause = false;
    m_stop = false;

    m_update_fb = false;
    m_update_projection = false;

    m_gui_mode = GUI_MODE_NONE;
    m_editor_gui = nullptr;

    m_glfw_time = 0.0;

    // shader setup (I don't like how this is organised)

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

    math::mat4 orto_proj = math::orthographic(fb_width, 0, fb_height , 0, 1.0f , -1.0f);
    glUseProgram(m_text_shader);
    glUniformMatrix4fv(m_text_proj_mat, 1, GL_FALSE, orto_proj.m);

    m_gui_shader = create_programme_from_files("../shaders/gui_vs.glsl",
                                                "../shaders/gui_fs.glsl");
    m_gui_proj_mat = glGetUniformLocation(m_gui_shader, "projection");

    glUseProgram(m_gui_shader);
    glUniformMatrix4fv(m_gui_proj_mat, 1, GL_FALSE, orto_proj.m);

    // debug overlay
    m_debug_overlay.reset(new DebugOverlay(fb_width, fb_height, this));

    // other gl stuff
    m_bg_r = 0.2;
    m_bg_g = 0.2;
    m_bg_b = 0.2;
    m_bg_a = 1.;

    glClearColor(m_bg_r, m_bg_g, m_bg_b, m_bg_a);

    m_rscene_acc_load_time = 0.0;
    m_rgui_acc_load_time = 0.0;
}


RenderContext::~RenderContext(){
    glDeleteShader(m_pb_notex_shader);
    glDeleteShader(m_pb_shader);
    glDeleteShader(m_text_shader);
    glDeleteShader(m_gui_shader);
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


void RenderContext::initImgui(){
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window_handler->getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 410");
    
    io.Fonts->AddFontFromFileTTF("../data/fonts/Liberastika-Regular.ttf", 16.0f);
    io.Fonts->Build();
}


void RenderContext::renderAttPoints(const BasePart* part, int& num_rendered, const math::mat4& body_transform){
    const std::vector<struct attachment_point>* att_points = part->getAttachmentPoints();

    if(att_points->size()){
        math::mat4 att_transform;
        math::vec3 point;

        if(part->hasParentAttPoint()){
            point = part->getParentAttachmentPoint()->point;
            att_transform = body_transform * math::translate(math::identity_mat4(), point);
            m_att_point_model->setMeshColor(math::vec4(0.0, 1.0, 0.0, 1.0));
            num_rendered += m_att_point_model->render(att_transform * m_att_point_scale);
        }

        if(part->hasFreeAttPoint()){
            point = part->getFreeAttachmentPoint()->point;
            att_transform = body_transform * math::translate(math::identity_mat4(), point);
            m_att_point_model->setMeshColor(math::vec4(0.0, 1.0, 1.0, 1.0));
            num_rendered += m_att_point_model->render(att_transform * m_att_point_scale);
        }

        for(uint j=0; j<att_points->size(); j++){
            point = att_points->at(j).point;
            att_transform = body_transform * math::translate(math::identity_mat4(), point);
            m_att_point_model->setMeshColor(math::vec4(1.0, 0.0, 0.0, 1.0));
            num_rendered += m_att_point_model->render(att_transform * m_att_point_scale);
        }
    }
}


void RenderContext::render(){
    int num_rendered = 0;
    std::chrono::steady_clock::time_point start_scene, end_scene_start_gui, end_gui;

    //clean up this shit


    glUseProgram(m_pb_notex_shader);
    glUniformMatrix4fv(m_pb_notex_view_mat, 1, GL_FALSE, m_camera->getViewMatrix().m);
    glUniformMatrix4fv(m_pb_notex_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    glUseProgram(m_pb_shader);
    glUniformMatrix4fv(m_pb_view_mat, 1, GL_FALSE, m_camera->getViewMatrix().m);
    glUniformMatrix4fv(m_pb_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    if(m_update_projection){
        math::mat4 projection = math::orthographic(m_fb_width, 0, m_fb_height, 0, 1.0f , -1.0f);
        glUseProgram(m_text_shader);
        glUniformMatrix4fv(m_text_proj_mat, 1, GL_FALSE, projection.m);
        glUseProgram(m_gui_shader);
        glUniformMatrix4fv(m_gui_proj_mat, 1, GL_FALSE, projection.m);

        m_debug_overlay->onFramebufferSizeUpdate(m_fb_width, m_fb_height);

        switch(m_gui_mode){
            case GUI_MODE_NONE:
            case GUI_MODE_EDITOR:
                m_editor_gui->onFramebufferSizeUpdate();
                break;
        }

        m_update_projection = false;
    }

    glClearColor(m_bg_r, m_bg_g, m_bg_b, m_bg_a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    start_scene = std::chrono::steady_clock::now();

    if(m_buffers->last_updated != none){
        if(m_buffers->last_updated == buffer_1){
            m_buffers->buffer1_lock.lock(); // extremely unlikely to not get the lock
            for(uint i=0; i<m_buffers->buffer1.size(); i++){
                BasePart* part = dynamic_cast<BasePart*>(m_buffers->buffer1.at(i).object_ptr.get());
                if(part){
                    renderAttPoints(part, num_rendered, m_buffers->buffer1.at(i).transform);
                }
                num_rendered += m_buffers->buffer1.at(i).object_ptr->render(m_buffers->buffer1.at(i).transform);
            }
            m_buffers->buffer1_lock.unlock();
        }
        else{
            m_buffers->buffer2_lock.lock();
            for(uint i=0; i<m_buffers->buffer2.size(); i++){
                BasePart* part = dynamic_cast<BasePart*>(m_buffers->buffer2.at(i).object_ptr.get());
                if(part){
                    renderAttPoints(part, num_rendered, m_buffers->buffer2.at(i).transform);
                }
                num_rendered += m_buffers->buffer2.at(i).object_ptr->render(m_buffers->buffer2.at(i).transform);
            }
            m_buffers->buffer2_lock.unlock();
        }
    }

    end_scene_start_gui = std::chrono::steady_clock::now();

    glDisable(GL_DEPTH_TEST);
    switch(m_gui_mode){
        case GUI_MODE_NONE:
            break;
        case GUI_MODE_EDITOR:
            m_editor_gui->render();
            break;
        default:
            std::cerr << "RenderContext::render - Warning, invalid GUI mode (" << m_gui_mode << ")" << std::endl;
            log("RenderContext::render - Warning, invalid GUI mode (", m_gui_mode, ")");
    }
    glEnable(GL_DEPTH_TEST);

    end_gui = std::chrono::steady_clock::now();

    std::chrono::duration<double, std::micro> load_time_scene = end_scene_start_gui - start_scene;
    std::chrono::duration<double, std::micro> load_time_gui = end_gui - end_scene_start_gui;
    m_rscene_acc_load_time += load_time_scene.count();
    m_rgui_acc_load_time += load_time_gui.count();

    testImgui();

    if(m_draw_overlay){
        m_debug_overlay->setRenderedObjects(num_rendered);
        m_debug_overlay->render();
    }
}


void RenderContext::setLightPosition(const math::vec3& pos) const{
    glUseProgram(m_pb_notex_shader);
    glUniform3fv(m_pb_notex_light_pos, 1, pos.v);
    glUseProgram(m_pb_shader);
    glUniform3fv(m_pb_light_pos, 1, pos.v);
}


GLuint RenderContext::getShader(int shader) const{
    switch(shader){
        case SHADER_PHONG_BLINN:
            return m_pb_shader;
        case SHADER_PHONG_BLINN_NO_TEXTURE:
            return m_pb_notex_shader;
        case SHADER_TEXT:
            return m_text_shader;
        case SHADER_GUI:
            return m_gui_shader;
        default:
            return 0;
    }
}


void RenderContext::setDebugOverlayTimes(double physics_load_time, double logic_load_time, double logic_sleep_time){
    m_debug_overlay->setTimes(physics_load_time, logic_load_time, logic_sleep_time);
}


void RenderContext::setAttPointModel(std::unique_ptr<Model>* att_point_model){
    m_att_point_model = std::move(*att_point_model);
}


GLuint RenderContext::getBoundShader() const{
    return m_bound_programme;
}


GLuint RenderContext::getBoundVao() const{
    return m_bound_vao;
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


void RenderContext::onFramebufferSizeUpdate(int width, int height){
    m_fb_width = width;
    m_fb_height = height;
    m_update_fb = true;
}


void RenderContext::start(){
    m_render_thread = std::thread(&RenderContext::run, this);
    log("RenderContext: starting rendering thread");
}


void RenderContext::run(){
    std::chrono::steady_clock::time_point start, end;
    double accumulated_load = 0.0, render_load_time, scene_render_time, gui_render_time;
    int ticks_since_last_update = 0;

    // this should be enough to transfer the opengl context to the current thread
    glfwMakeContextCurrent(m_window_handler->getWindow());

    while(!m_stop){
        start = std::chrono::steady_clock::now();
        if(m_update_fb){
            m_update_fb = false;
            m_update_projection = true;
            glViewport(0, 0, m_fb_width, m_fb_height);
        }

        render();

        end = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::micro> load_time = end - start;
        accumulated_load += load_time.count();
        if(ticks_since_last_update == 60){
            render_load_time = accumulated_load / 60000.0;
            scene_render_time = m_rscene_acc_load_time / 60000.0;
            gui_render_time = m_rgui_acc_load_time / 60000.0;
            m_debug_overlay->setRenderTimes(render_load_time, scene_render_time, gui_render_time);
            accumulated_load = 0.0;
            m_rscene_acc_load_time = 0.0;
            m_rgui_acc_load_time = 0.0;
            ticks_since_last_update = 0;
        }
        else{
            ticks_since_last_update++;
        }

        glfwSwapBuffers(m_window_handler->getWindow());
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}


void RenderContext::stop(){
    m_stop = true;
    m_render_thread.join();
    log("RenderContext: stopping rendering thread");
}


void RenderContext::pause(bool pause){
    m_pause = pause;
}


void RenderContext::toggleDebugOverlay(){
    m_draw_overlay = !m_draw_overlay;
}


void RenderContext::getDefaultFbSize(float& width, float& height) const{
    width = m_fb_width;
    height = m_fb_height;
}


void RenderContext::setEditorGUI(BaseGUI* editor_ptr){
    m_editor_gui = editor_ptr;
}


void RenderContext::setEditorMode(short mode){
    m_gui_mode = mode;
}


void RenderContext::testImgui(){
    // testing imgui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        static float f = 0.0f;
        static int counter = 0;
        static bool a = false, b = true;
        static float col[] = {0.5, 0.5, 0.5};

        //ImGui::SetNextWindowPos(ImVec2(500, 500));
        ImGui::Begin("Hello, world!");

        ImGui::Text("This is some useful text.");
        ImGui::Checkbox("Demo Window", &a);
        ImGui::Checkbox("Another Window", &b);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        ImGui::ColorEdit3("clear color", col);

        if (ImGui::Button("Button")){
            counter++;
        }
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

