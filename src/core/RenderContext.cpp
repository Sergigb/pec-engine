#include <chrono>
#include <mutex>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#define BT_USE_DOUBLE_PRECISION
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include "RenderContext.hpp"
#include "Camera.hpp"
#include "WindowHandler.hpp"
#include "buffers.hpp"
#include "log.hpp"
#include "DebugDrawer.hpp"
#include "Physics.hpp"
#include "BaseApp.hpp"
#include "utils/gl_utils.hpp"
#include "AssetManager.hpp"
#include "Player.hpp"
#include "../assets/PlanetarySystem.hpp"
#include "../assets/Planet.hpp"
#include "../assets/BasePart.hpp"
#include "../assets/Model.hpp"
#include "../GUI/Text2D.hpp"
#include "../GUI/BaseGUI.hpp"
#include "../GUI/DebugOverlay.hpp"


RenderContext::RenderContext(BaseApp* app){
    m_camera = app->m_camera.get();
    m_window_handler = app->m_window_handler.get();
    m_buffers = &app->m_buffers;
    m_app = app;
    m_draw_overlay = false;
    m_debug_draw = false;
    m_update_shaders = false;
    m_light_position = math::vec3(0.0f, 0.0f, 0.0f);

    int fb_width, fb_height;
    m_window_handler->getFramebufferSize(fb_width, fb_height);
    m_fb_width = fb_width;
    m_fb_height = fb_height;

    initGl();
    log_gl_params();
    initImgui();

    m_att_point_scale = math::identity_mat4();
    m_att_point_scale.m[0] = 0.25;
    m_att_point_scale.m[5] = 0.25;
    m_att_point_scale.m[10] = 0.25;

    m_bound_vao = 0;
    m_bound_programme = 0;

    m_stop = false;

    m_update_fb = false;
    m_update_projection = false;

    m_editor_gui = nullptr;
    m_default_atlas = nullptr;

    m_glfw_time = 0.0;

    loadShaders();

    // debug overlay
    m_debug_overlay.reset(new DebugOverlay(fb_width, fb_height, this));

    // other gl stuff
    m_color_clear = math::vec4(0.428, 0.706f, 0.751f, 1.0f);

    glClearColor(m_color_clear.v[0], m_color_clear.v[1], m_color_clear.v[2], m_color_clear.v[3]);

    m_rscene_acc_load_time = 0.0;
    m_rgui_acc_load_time = 0.0;

    check_gl_errors(true, "RenderContext::RenderContext");
}


void RenderContext::setDebugDrawer(Physics* physics){
    m_physics = physics;
    m_debug_drawer.reset(new DebugDrawer(this));
    btDiscreteDynamicsWorld* d_world = m_physics->getDynamicsWorld();
    d_world->setDebugDrawer(m_debug_drawer.get());
}


RenderContext::~RenderContext(){
    glDeleteShader(m_pb_notex_shader);
    glDeleteShader(m_pb_shader);
    glDeleteShader(m_text_shader);
    glDeleteShader(m_gui_shader);
    glDeleteShader(m_planet_shader);
    glDeleteShader(m_debug_shader);
}


void RenderContext::loadShaders(){
    // shader setup (I don't like how this is organised)

    m_debug_shader = create_programme_from_files("../shaders/debug_vs.glsl",
                                                 "../shaders/debug_fs.glsl");
    log_programme_info(m_debug_shader);
    m_debug_view_mat = glGetUniformLocation(m_debug_shader, "view");
    m_debug_proj_mat = glGetUniformLocation(m_debug_shader, "proj");
    m_debug_color_location = glGetUniformLocation(m_debug_shader, "line_color");

    m_planet_shader = create_programme_from_files("../shaders/planet_vs.glsl",
                                                  "../shaders/planet_fs.glsl");
    log_programme_info(m_planet_shader);
    m_planet_view_mat = glGetUniformLocation(m_planet_shader, "view");
    m_planet_proj_mat = glGetUniformLocation(m_planet_shader, "proj");
    m_planet_light_pos = glGetUniformLocation(m_planet_shader, "light_pos");
    m_planet_relative_pos = glGetUniformLocation(m_planet_shader, "relative_planet");

    glUseProgram(m_planet_shader);
    glUniformMatrix4fv(m_planet_view_mat, 1, GL_FALSE, m_camera->getViewMatrix().m);
    glUniformMatrix4fv(m_planet_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);
    glUniform3fv(m_planet_light_pos, 1, math::vec3(0.0, 0.0, 0.0).v);

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

    math::mat4 orto_proj = math::orthographic(m_fb_width, 0, m_fb_height , 0, 1.0f , -1.0f);
    glUseProgram(m_text_shader);
    glUniformMatrix4fv(m_text_proj_mat, 1, GL_FALSE, orto_proj.m);

    m_gui_shader = create_programme_from_files("../shaders/gui_vs.glsl",
                                                "../shaders/gui_fs.glsl");
    m_gui_proj_mat = glGetUniformLocation(m_gui_shader, "projection");

    glUseProgram(m_gui_shader);
    glUniformMatrix4fv(m_gui_proj_mat, 1, GL_FALSE, orto_proj.m);

    check_gl_errors(true, "RenderContext::loadShaders");
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

    check_gl_errors(true, "RenderContext::initGl");
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


void RenderContext::renderAttPoints(const BasePart* part, const math::mat4& body_transform){
    const std::vector<struct attachment_point>& att_points = part->getAttachmentPoints();

    math::mat4 att_transform;

    if(part->hasParentAttPoint()){
        const math::vec3& point = part->getParentAttachmentPoint().point;
        att_transform = body_transform * math::translate(math::identity_mat4(), point);
        m_att_point_model->setMeshColor(math::vec4(0.0, 1.0, 0.0, 1.0));
        m_att_point_model->render(att_transform * m_att_point_scale);
    }

    /*if(part->hasFreeAttPoint()){
        point = part->getFreeAttachmentPoint()->point;
        att_transform = body_transform * math::translate(math::identity_mat4(), point);
        m_att_point_model->setMeshColor(math::vec4(0.0, 1.0, 1.0, 1.0));
        num_rendered += m_att_point_model->render(att_transform * m_att_point_scale);
    }*/

    for(uint j=0; j<att_points.size(); j++){
        const math::vec3& point = att_points.at(j).point;
        att_transform = body_transform * math::translate(math::identity_mat4(), point);
        m_att_point_model->setMeshColor(math::vec4(1.0, 0.0, 0.0, 1.0));
        m_att_point_model->render(att_transform * m_att_point_scale);
    }
}


int RenderContext::renderSceneEditor(){
    int num_rendered = 0;
    struct render_buffer* rbuf;

    if(m_buffers->last_updated != none){
        if(m_buffers->last_updated == buffer_1){
            m_buffers->buffer_1.buffer_lock.lock();
            rbuf = &m_buffers->buffer_1;
        }
        else{
            m_buffers->buffer_2.buffer_lock.lock();
            rbuf = &m_buffers->buffer_2;
        }
        num_rendered = renderObjects(true, &rbuf->buffer, &rbuf->view_mat);
        rbuf->buffer_lock.unlock();
    }

    return num_rendered;
}


int RenderContext::renderSceneUniverse(){
    int num_rendered = 0;
    struct render_buffer* rbuf;

    if(m_buffers->last_updated != none){
        if(m_buffers->last_updated == buffer_1){
            m_buffers->buffer_1.buffer_lock.lock();
            rbuf = &m_buffers->buffer_1;
        }
        else{
            m_buffers->buffer_2.buffer_lock.lock();
            rbuf = &m_buffers->buffer_2;
        }
        num_rendered = renderObjects(false, &rbuf->buffer, &rbuf->view_mat);

        for(uint i=0; i < rbuf->planet_buffer.size(); i++){
            planet_transform& tr = rbuf->planet_buffer.at(i);
            tr.planet_ptr->render(rbuf->cam_origin, tr.transform);
        }

        rbuf->buffer_lock.unlock();
    }

    return num_rendered;
}


int RenderContext::renderObjects(bool render_att_points, const std::vector<object_transform>* buff, const math::mat4* view_mat){
    int num_rendered = 0;

    glUseProgram(m_pb_notex_shader);
    glUniformMatrix4fv(m_pb_notex_view_mat, 1, GL_FALSE, view_mat->m);
    glUniformMatrix4fv(m_pb_notex_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    glUseProgram(m_pb_shader);
    glUniformMatrix4fv(m_pb_view_mat, 1, GL_FALSE, view_mat->m);
    glUniformMatrix4fv(m_pb_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    glUseProgram(m_planet_shader);
    glUniformMatrix4fv(m_planet_view_mat, 1, GL_FALSE, view_mat->m);
    glUniformMatrix4fv(m_planet_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    for(uint i=0; i<buff->size(); i++){
        if(render_att_points){
            BasePart* part = dynamic_cast<BasePart*>(buff->at(i).object_ptr.get());
            if(part){
                renderAttPoints(part, buff->at(i).transform);
            }
        }
        num_rendered += buff->at(i).object_ptr->render(buff->at(i).transform);
    }

    if(m_debug_draw){
        renderBulletDebug(view_mat);
    }

    return num_rendered;
}


void RenderContext::renderBulletDebug(const math::mat4* view_mat){
    glUseProgram(m_debug_shader);
    glUniformMatrix4fv(m_debug_view_mat, 1, GL_FALSE, view_mat->m);
    glUniformMatrix4fv(m_debug_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    const dmath::vec3& cam_position = m_camera->getCamPosition();
    m_debug_drawer->getReady();
    m_debug_drawer->setCameraCenter(btVector3(cam_position.v[0], cam_position.v[1], cam_position.v[2]));
    m_physics->getDynamicsWorld()->debugDrawWorld();
}


void RenderContext::renderPlanetarium(){
    struct render_buffer* rbuf;

    if(m_buffers->last_updated != none){
        if(m_buffers->last_updated == buffer_1){
            m_buffers->buffer_1.buffer_lock.lock();
            rbuf = &m_buffers->buffer_1;
        }
        else{
            m_buffers->buffer_2.buffer_lock.lock();
            rbuf = &m_buffers->buffer_2;
        }

        m_app->m_asset_manager->m_planetary_system->updateRenderBuffers(m_app->m_physics->getCurrentTime());
        renderPlanetariumOrbits(rbuf->planet_buffer, rbuf->view_mat);

        rbuf->buffer_lock.unlock();
    }
}


void RenderContext::renderPlanetariumOrbits(const std::vector<planet_transform>& buff, const math::mat4& view_mat){
    useProgram(SHADER_DEBUG);
    glUniformMatrix4fv(m_debug_view_mat, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(m_debug_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(uint i=0; i < buff.size(); i++){
        Planet* current = buff.at(i).planet_ptr;

        if(current->getId() == m_app->m_player->getPlanetariumSelectedPlanet()){
            glUniform3f(m_debug_color_location, 0.0, 1.0, 0.0);
        }
        else{
            glUniform3f(m_debug_color_location, 1.0, 0.0, 0.0);
        }

        current->renderOrbit();
    }
}


void RenderContext::render(){
    int num_rendered = 0;

    std::chrono::steady_clock::time_point start_scene, end_scene_start_gui, end_gui_start_imgui, end_imgui;

    //clean up this shit

    if(m_update_shaders){
        m_update_shaders = false;
        loadShaders();
        log("RenderContext::render - shaders reloaded");
        std::cout << "shaders reloaded" << std::endl;
    }

    if(m_update_projection){
        math::mat4 projection = math::orthographic(m_fb_width, 0, m_fb_height, 0, 1.0f , -1.0f);
        glUseProgram(m_text_shader);
        glUniformMatrix4fv(m_text_proj_mat, 1, GL_FALSE, projection.m);
        glUseProgram(m_gui_shader);
        glUniformMatrix4fv(m_gui_proj_mat, 1, GL_FALSE, projection.m);

        m_debug_overlay->onFramebufferSizeUpdate(m_fb_width, m_fb_height);

        switch(m_app->getGUIMode()){
            case GUI_MODE_NONE:
            case GUI_MODE_EDITOR:
                m_editor_gui->onFramebufferSizeUpdate();
                break;
        }

        m_update_projection = false;
    }

    glClearColor(m_color_clear.v[0], m_color_clear.v[1], m_color_clear.v[2], m_color_clear.v[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    start_scene = std::chrono::steady_clock::now();

    setLightPositionRender();

    switch(m_app->getRenderState()){
        case RENDER_NOTHING:
            break;
        case RENDER_EDITOR:
            num_rendered = renderSceneEditor();
            break;
        case RENDER_UNIVERSE:
            num_rendered = renderSceneUniverse();
            break;
        case RENDER_PLANETARIUM:
            renderPlanetarium();
            break;
        default:
            std::cerr << "RenderContext::render: Warning, invalid render state value (" << (int)m_app->getRenderState() << ")" << std::endl;
            log("RenderContext::render: Warning, invalid render state value (", (int)m_app->getRenderState(), ")");
    }
    

    end_scene_start_gui = std::chrono::steady_clock::now();

    glDisable(GL_DEPTH_TEST);
    switch(m_app->getGUIMode()){
        case GUI_MODE_NONE:
            break;
        case GUI_MODE_EDITOR:
            m_editor_gui->render();
            break;
        default:
            std::cerr << "RenderContext::render - Warning, invalid GUI mode (" << m_app->getGUIMode() << ")" << std::endl;
            log("RenderContext::render - Warning, invalid GUI mode (", m_app->getGUIMode(), ")");
    }
    glEnable(GL_DEPTH_TEST);

    end_gui_start_imgui = std::chrono::steady_clock::now();

    renderImGui();
    renderNotifications();

    end_imgui = std::chrono::steady_clock::now();

    std::chrono::duration<double, std::micro> load_time_scene = end_scene_start_gui - start_scene;
    std::chrono::duration<double, std::micro> load_time_gui = end_gui_start_imgui - end_scene_start_gui;
    std::chrono::duration<double, std::micro> load_time_imgui = end_imgui - end_gui_start_imgui;
    m_rscene_acc_load_time += load_time_scene.count();
    m_rgui_acc_load_time += load_time_gui.count();
    m_rimgui_acc_load_time += load_time_imgui.count();

    if(m_draw_overlay){
        glDisable(GL_DEPTH_TEST);
        m_debug_overlay->setRenderedObjects(num_rendered);
        m_debug_overlay->render();
        glEnable(GL_DEPTH_TEST);
    }

    check_gl_errors(true, "RenderContext::render - fix them :P");
}


void RenderContext::setLightPosition(const math::vec3& pos){
    m_light_position = pos;
}


void RenderContext::setLightPositionRender(){
    glUseProgram(m_pb_notex_shader);
    glUniform3fv(m_pb_notex_light_pos, 1, m_light_position.v);
    glUseProgram(m_pb_shader);
    glUniform3fv(m_pb_light_pos, 1, m_light_position.v);
    glUseProgram(m_planet_shader);
    glUniform3fv(m_planet_light_pos, 1, m_light_position.v);
}


void RenderContext::setDebugOverlayTimes(double physics_load_time, double logic_load_time, double logic_sleep_time){
    m_debug_overlay->setTimes(physics_load_time, logic_load_time, logic_sleep_time);
}


void RenderContext::setAttPointModel(std::unique_ptr<Model>* att_point_model){
    m_att_point_model = std::move(*att_point_model);
}


void RenderContext::useProgram(int shader) const{
    if(shader == m_bound_programme){
        return;
    }
    switch(shader){
        case SHADER_PHONG_BLINN:
            glUseProgram(m_pb_shader);
            break;
        case SHADER_PHONG_BLINN_NO_TEXTURE:
            glUseProgram(m_pb_notex_shader);
            break;
        case SHADER_TEXT:
            glUseProgram(m_text_shader);
            break;
        case SHADER_GUI:
            glUseProgram(m_gui_shader);
            break;
        case SHADER_PLANET:
            glUseProgram(m_planet_shader);
            break;
        case SHADER_DEBUG:
            glUseProgram(m_debug_shader);
            break;
        default:
            std::cerr << "RenderContext::useProgram - wrong shader value " << shader << std::endl;
            log("RenderContext::useProgram - wrong shader value ", shader);
            return;
    }
}


GLuint RenderContext::getUniformLocation(int shader, const char* location) const{
    switch(shader){
        case SHADER_PHONG_BLINN:
            return glGetUniformLocation(m_pb_shader, location);
        case SHADER_PHONG_BLINN_NO_TEXTURE:
            return glGetUniformLocation(m_pb_notex_shader, location);
        case SHADER_TEXT:
            return glGetUniformLocation(m_text_shader, location);
        case SHADER_GUI:
            return glGetUniformLocation(m_gui_shader, location);
        case SHADER_PLANET:
            return glGetUniformLocation(m_planet_shader, location);
        case SHADER_DEBUG:
            return glGetUniformLocation(m_debug_shader, location);
        default:
            std::cerr << "RenderContext::getUniformLocation - wrong shader value " << shader << std::endl;
            log("RenderContext::getUniformLocation - wrong shader value ", shader);
    }
    return -1;
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
    double accumulated_load = 0.0, render_load_time, scene_render_time, gui_render_time, imgui_render_time;
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
            imgui_render_time = m_rimgui_acc_load_time / 60000.0;
            m_debug_overlay->setRenderTimes(render_load_time, scene_render_time, gui_render_time, imgui_render_time);
            accumulated_load = 0.0;
            m_rscene_acc_load_time = 0.0;
            m_rgui_acc_load_time = 0.0;
            m_rimgui_acc_load_time = 0.0;
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


void RenderContext::toggleDebugOverlay(){
    m_draw_overlay = !m_draw_overlay;

    if(m_draw_overlay){
        addNotification(L"Debug overlay enabled");
    }
    else{
        addNotification(L"Debug overlay disabled");
    }
}


void RenderContext::toggleDebugDraw(){
    m_debug_draw = !m_debug_draw;
    if(m_debug_draw){
        addNotification(L"Debug drawing activated");
    }
    else{
        addNotification(L"Debug drawing deactivated");
    }
}


void RenderContext::getDefaultFbSize(float& width, float& height) const{
    width = m_fb_width;
    height = m_fb_height;
}


void RenderContext::setEditorGUI(BaseGUI* editor_ptr){
    m_editor_gui = editor_ptr;
}


void RenderContext::renderImGui(){
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if(m_buffers->last_updated != none){
        if(m_buffers->last_updated == buffer_1){
            m_buffers->buffer_1.buffer_lock.lock();
            for(uint i=0; i<m_buffers->buffer_1.buffer.size(); i++){
                m_buffers->buffer_1.buffer.at(i).object_ptr.get()->renderOther();
            }
            m_buffers->buffer_1.buffer_lock.unlock();
        }
        else{
            m_buffers->buffer_2.buffer_lock.lock();
            for(uint i=0; i<m_buffers->buffer_2.buffer.size(); i++){
                m_buffers->buffer_2.buffer.at(i).object_ptr.get()->renderOther();
            }
            m_buffers->buffer_2.buffer_lock.unlock();
        }
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


bool RenderContext::imGuiWantCaptureMouse() const{
    ImGuiIO& io = ImGui::GetIO();

    return io.WantCaptureMouse;
}


bool RenderContext::imGuiWantCaptureKeyboard() const{
    ImGuiIO& io = ImGui::GetIO();

    return io.WantCaptureKeyboard;
}


void RenderContext::contextUpdatePlanetRenderer(){
    if(m_update_fb){
        m_update_fb = false;
        m_update_projection = true;
        glViewport(0, 0, m_fb_width, m_fb_height);
    }

    if(m_update_projection){
        math::mat4 projection = math::orthographic(m_fb_width, 0, m_fb_height, 0, 1.0f , -1.0f);
        glUseProgram(m_text_shader);
        glUniformMatrix4fv(m_text_proj_mat, 1, GL_FALSE, projection.m);
        glUseProgram(m_gui_shader);
        glUniformMatrix4fv(m_gui_proj_mat, 1, GL_FALSE, projection.m);

        m_debug_overlay->onFramebufferSizeUpdate(m_fb_width, m_fb_height);

        m_update_projection = false;
    }

    glClearColor(m_color_clear.v[0], m_color_clear.v[1], m_color_clear.v[2], m_color_clear.v[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_pb_notex_shader);
    glUniformMatrix4fv(m_pb_notex_view_mat, 1, GL_FALSE, m_camera->getCenteredViewMatrix().m);
    glUniformMatrix4fv(m_pb_notex_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    glUseProgram(m_pb_shader);
    glUniformMatrix4fv(m_pb_view_mat, 1, GL_FALSE, m_camera->getCenteredViewMatrix().m);
    glUniformMatrix4fv(m_pb_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    glUseProgram(m_planet_shader);
    glUniformMatrix4fv(m_planet_view_mat, 1, GL_FALSE, m_camera->getCenteredViewMatrix().m);
    glUniformMatrix4fv(m_planet_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    setLightPositionRender();

    if(m_update_shaders){
        m_update_shaders = false;
        loadShaders();
    }

    if(m_draw_overlay){
        glDisable(GL_DEPTH_TEST);
        m_debug_overlay->render();
        glEnable(GL_DEPTH_TEST);
    }
}


void RenderContext::reloadShaders(){
    m_update_shaders = true;

    addNotification(L"Shaders reloaded");
}


void RenderContext::addNotification(const wchar_t* string, int ttl){
    notifications.emplace_back(notification{std::wstring(string), ttl});
}


void RenderContext::renderNotifications(){
    m_notification_text->clearStrings();
    if(!m_default_atlas){
        return;
    }

    std::vector<notification>::iterator it = notifications.begin();
    uint disp = 0.0;
    while(it != notifications.end()){
        disp += 20;
        m_notification_text->addString(it->string.c_str(), m_fb_width / 2, (m_fb_height / 2) - disp, 1.0, STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);

        it->ttl -= 1;
        if(it->ttl < 0){
            it = notifications.erase(it);
        }
        else{
            it++;
        }
    }
    m_notification_text->render();
}


void RenderContext::setDefaultFontAtlas(const FontAtlas* atlas){
    m_default_atlas = atlas;
    m_notification_text.reset(new Text2D(m_fb_width, m_fb_height, {0.f, 1.f, 0.f}, m_default_atlas, this));
}

