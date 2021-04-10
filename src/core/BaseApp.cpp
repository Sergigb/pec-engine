#include "BaseApp.hpp"

#include "Camera.hpp"
#include "Input.hpp"
#include "WindowHandler.hpp"
#include "Frustum.hpp"
#include "Physics.hpp"
#include "RenderContext.hpp"
#include "AssetManager.hpp"
#include "Player.hpp"
#include "../assets/Model.hpp"


BaseApp::BaseApp(){
    init(640, 480);
}


BaseApp::BaseApp(int gl_width, int gl_height){
    init(gl_width, gl_height);
}


BaseApp::~BaseApp(){
}


void BaseApp::init(int gl_width, int gl_height){
    m_input.reset(new Input());
    m_camera.reset(new Camera(dmath::vec3(-0.0f, 50.0f, 50.0f), 67.0f, (float)gl_width / (float)gl_height , 0.1f, 63000000.0f, m_input.get()));
    m_camera->setSpeed(10.f);
    m_window_handler.reset(new WindowHandler(gl_width, gl_height, m_input.get(), m_camera.get()));
    m_camera->setWindowHandler(m_window_handler.get());
    m_frustum.reset(new Frustum());
    m_render_context.reset(new RenderContext(this));
    m_window_handler->setRenderContext(m_render_context.get());
    m_physics.reset(new Physics(btVector3(0, 0, 0), &m_thread_monitor));
    m_render_context->setDebugDrawer(m_physics.get());
    m_asset_manager.reset(new AssetManager(this));
    m_player.reset(new Player(m_camera.get(), m_asset_manager.get(), m_input.get()));

    std::unique_ptr<Model> att_model(new Model("../data/sphere.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, m_frustum.get(), m_render_context.get(), math::vec3(1.0, 0.0, 0.0)));
    m_render_context->setAttPointModel(&att_model);

    m_buffers.last_updated = none;

    m_gui_mode = 0;
    m_render_state = 0;
}


void BaseApp::run(){
    while (!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        //m_render_context->render(true);
       // glfwSwapBuffers(m_window_handler->getWindow());
    }
    m_window_handler->terminate();
}

short BaseApp::getGUIMode() const{
    return m_gui_mode;
}


short BaseApp::getRenderState() const{
    return m_render_state;
}
