#include "BaseApp.hpp"

#include "Camera.hpp"
#include "Input.hpp"
#include "WindowHandler.hpp"
#include "Frustum.hpp"
#include "BtWrapper.hpp"
#include "RenderContext.hpp"
#include "AssetManager.hpp"
#include "Player.hpp"
#include "Model.hpp"


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
    m_render_context.reset(new RenderContext(m_camera.get(), m_window_handler.get(), &m_buffers));
    m_window_handler->setRenderContext(m_render_context.get());
    m_bt_wrapper.reset(new BtWrapper(btVector3(0, 0, 0), &m_thread_monitor));
    m_render_context->setDebugDrawer(m_bt_wrapper.get());
    m_asset_manager.reset(new AssetManager(m_render_context.get(), m_frustum.get(), m_bt_wrapper.get(), &m_buffers, m_camera.get()));
    m_player.reset(new Player(m_camera.get(), m_asset_manager.get(), m_input.get()));

    std::unique_ptr<Model> att_model(new Model("../data/sphere.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, m_frustum.get(), m_render_context.get(), math::vec3(1.0, 0.0, 0.0)));
    m_render_context->setAttPointModel(&att_model);

    m_buffers.last_updated = none;
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

