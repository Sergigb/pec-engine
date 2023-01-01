#include <mutex>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#define BT_USE_DOUBLE_PRECISION
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include "utils/gl_utils.hpp"
#include "RenderContext.hpp"
#include "Camera.hpp"
#include "WindowHandler.hpp"
#include "buffers.hpp"
#include "log.hpp"
#include "DebugDrawer.hpp"
#include "Physics.hpp"
#include "BaseApp.hpp"
#include "AssetManager.hpp"
#include "Player.hpp"
#include "timing.hpp"
#include "../assets/PlanetarySystem.hpp"
#include "../assets/Planet.hpp"
#include "../assets/BasePart.hpp"
#include "../assets/Model.hpp"
#include "../GUI/Text2D.hpp"
#include "../GUI/BaseGUI.hpp"
#include "../GUI/DebugOverlay.hpp"
#include "../renderers/BaseRenderer.hpp"


RenderContext::RenderContext(BaseApp* app){
    m_camera = app->getCamera();
    m_window_handler = app->getWindowHandler();
    m_buffers = app->getRenderBuffers();
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

    m_bound_vao = 0;
    m_bound_programme = 0;

    m_stop = false;

    m_update_fb = false;
    m_update_projection = false;

    m_editor_gui = nullptr;
    m_planetarium_gui = nullptr;
    m_default_atlas = nullptr;

    m_planetarium_renderer = nullptr;
    m_simulation_renderer = nullptr;
    m_editor_renderer = nullptr;

    m_glfw_time = 0.0;

    loadShaders();

    // debug overlay
    m_debug_overlay.reset(new DebugOverlay(fb_width, fb_height, this));

    // other gl stuff
    m_color_clear = math::vec4(0.428, 0.706f, 0.751f, 1.0f);

    glClearColor(m_color_clear.v[0], m_color_clear.v[1], m_color_clear.v[2], m_color_clear.v[3]);

    check_gl_errors(true, "RenderContext::RenderContext");
}


void RenderContext::setDebugDrawer(){
    m_physics = m_app->getPhysics();
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
    glDeleteShader(m_sprite_shader);
    glDeleteShader(m_tnl_shader);
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

    m_text_shader = create_programme_from_files("../shaders/2D_vs.glsl",
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

    m_sprite_shader = create_programme_from_files("../shaders/2D_vs.glsl",
                                                  "../shaders/sprite_fs.glsl");
    m_sprite_proj_mat = glGetUniformLocation(m_sprite_shader, "projection");

    glUseProgram(m_sprite_shader);
    glUniformMatrix4fv(m_sprite_proj_mat, 1, GL_FALSE, orto_proj.m);

    m_tnl_shader = create_programme_from_files("../shaders/texture_no_light_vs.glsl",
                                               "../shaders/texture_no_light_fs.glsl");
    log_programme_info(m_tnl_shader);
    m_tnl_view_mat = glGetUniformLocation(m_tnl_shader, "view");
    m_tnl_proj_mat = glGetUniformLocation(m_tnl_shader, "proj");

    glUseProgram(m_tnl_shader);
    glUniformMatrix4fv(m_tnl_view_mat, 1, GL_FALSE, m_camera->getViewMatrix().m);
    glUniformMatrix4fv(m_tnl_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    check_gl_errors(true, "RenderContext::loadShaders");
}


void RenderContext::initGl(){
    log("RenderContext::initGl: Starting GLEW");
    glewExperimental = GL_TRUE;
    glewInit();

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "RenderContext::initGl: Renderer: " << renderer << std::endl;
    std::cout << "RenderContext::initGl: OpenGL version supported: " << version << std::endl;
    log("RenderContext::initGl: Renderer: ", renderer, ", using OpenGL version: ", version);

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
        log("RenderContext::initGl: MSAA is available with ", samples, " samples");
    else
        log("RenderContext::initGl: MSAA is unavailable");

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


void RenderContext::renderBulletDebug(const math::mat4& view_mat){
    glUseProgram(m_debug_shader);
    glUniformMatrix4fv(m_debug_view_mat, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(m_debug_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    const dmath::vec3& cam_position = m_camera->getCamPosition();
    m_debug_drawer->getReady();
    m_debug_drawer->setCameraCenter(btVector3(cam_position.v[0], cam_position.v[1], cam_position.v[2]));
    m_physics->getDynamicsWorld()->debugDrawWorld();
}


void RenderContext::render(){
    int num_rendered = 0;

    //clean up this shit

    if(m_update_shaders){
        m_update_shaders = false;
        loadShaders();
        log("RenderContext::render: shaders reloaded");
        std::cout << "RenderContext::initGl: shaders reloaded" << std::endl;
    }

    if(m_update_projection){
        math::mat4 projection = math::orthographic(m_fb_width, 0, m_fb_height, 0, 1.0f , -1.0f);
        glUseProgram(m_text_shader);
        glUniformMatrix4fv(m_text_proj_mat, 1, GL_FALSE, projection.m);
        glUseProgram(m_gui_shader);
        glUniformMatrix4fv(m_gui_proj_mat, 1, GL_FALSE, projection.m);
        glUseProgram(m_sprite_shader);
        glUniformMatrix4fv(m_sprite_proj_mat, 1, GL_FALSE, projection.m);

        m_debug_overlay->onFramebufferSizeUpdate(m_fb_width, m_fb_height);

        switch(m_app->getGUIMode()){
            case GUI_MODE_NONE:
            case GUI_MODE_EDITOR:
                m_editor_gui->onFramebufferSizeUpdate();
                break;
            case GUI_MODE_PLANETARIUM:
                m_planetarium_gui->onFramebufferSizeUpdate();
                break;
        }

        m_update_projection = false;
    }

    glClearColor(m_color_clear.v[0], m_color_clear.v[1], m_color_clear.v[2], m_color_clear.v[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_timing.register_tp(TP_RENDER_START);

    setLightPositionRender();

    // scene render, should we make a separate function?
    if(m_buffers->last_updated != none){
        struct render_buffer* rbuf;
        if(m_buffers->last_updated == buffer_1){
            m_buffers->buffer_1.buffer_lock.lock();
            rbuf = &m_buffers->buffer_1;
        }
        else{
            m_buffers->buffer_2.buffer_lock.lock();
            rbuf = &m_buffers->buffer_2;
        }
        switch(m_app->getRenderState()){
            case RENDER_NOTHING:
                break;
            case RENDER_EDITOR:
                num_rendered = m_editor_renderer->render(rbuf);
                break;
            case RENDER_SIMULATION:
                num_rendered = m_simulation_renderer->render(rbuf);
                break;
            case RENDER_PLANETARIUM:
                m_planetarium_renderer->render(rbuf);
                break;
            default:
                std::cerr << "RenderContext::render: Warning, invalid render state value (" << (int)m_app->getRenderState() << ")" << std::endl;
                log("RenderContext::render: Warning, invalid render state value (", (int)m_app->getRenderState(), ")");
        }
        if(m_debug_draw && (RENDER_EDITOR | RENDER_SIMULATION))
            renderBulletDebug(rbuf->view_mat);

        rbuf->buffer_lock.unlock();
    }

    m_timing.register_tp(TP_SCENE_END);

    glDisable(GL_DEPTH_TEST);
    switch(m_app->getGUIMode()){
        case GUI_MODE_NONE:
            break;
        case GUI_MODE_EDITOR:
            m_editor_gui->render();
            break;
        case GUI_MODE_PLANETARIUM:
            m_planetarium_gui->render();
            break;
        default:
            std::cerr << "RenderContext::render: Warning, invalid GUI mode (" << m_app->getGUIMode() << ")" << std::endl;
            log("RenderContext::render: Warning, invalid GUI mode (", m_app->getGUIMode(), ")");
    }
    glEnable(GL_DEPTH_TEST);

    m_timing.register_tp(TP_GUI_END);

    renderImGui();
    renderNotifications();

    m_timing.register_tp(TP_RENDER_END);

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


DebugOverlay* RenderContext::getDebugOverlay(){
    return m_debug_overlay.get();
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
        case SHADER_SPRITE:
            glUseProgram(m_sprite_shader);
            break;
        case SHADER_TEXTURE_NO_LIGHT:
            glUseProgram(m_tnl_shader);
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
        case SHADER_SPRITE:
            return glGetUniformLocation(m_sprite_shader, location);
        case SHADER_TEXTURE_NO_LIGHT:
            return glGetUniformLocation(m_tnl_shader, location);
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
    // this should be enough to transfer the opengl context to the current thread
    glfwMakeContextCurrent(m_window_handler->getWindow());

    while(!m_stop){
        if(m_update_fb){
            m_update_fb = false;
            m_update_projection = true;
            glViewport(0, 0, m_fb_width, m_fb_height);
        }

        render();
        m_timing.update();
        m_debug_overlay->setRenderTimes(m_timing);

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


void RenderContext::setGUI(BaseGUI* gui_ptr, short gui){
    switch(gui){
        case GUI_MODE_EDITOR:
            m_editor_gui = gui_ptr;
            break;
        case GUI_MODE_PLANETARIUM:
            m_planetarium_gui = gui_ptr;
            break;
        default:
            std::cerr << "RenderContext::setGUI: invalid GUI mode: " << gui << std::endl;
            log("RenderContext::setGUI: invalid GUI mode ", gui);
    }
}


void RenderContext::setRenderer(BaseRenderer* rend_ptr, short render_state){
    switch(render_state){
        case RENDER_SIMULATION:
            m_simulation_renderer = rend_ptr;
            break;
        case RENDER_PLANETARIUM:
            m_planetarium_renderer = rend_ptr;
            break;  
        case RENDER_EDITOR:
            m_editor_renderer = rend_ptr;
            break;
        default:
            std::cerr << "RenderContext::setRenderer: invalid render state: " 
                      << render_state << std::endl;
            log("RenderContext::setRenderer: invalid render state ", render_state);
    }
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
        glUseProgram(m_sprite_shader);
        glUniformMatrix4fv(m_sprite_proj_mat, 1, GL_FALSE, projection.m);

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

