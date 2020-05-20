#include "BaseApp.hpp"


BaseApp::BaseApp(){
    init(640, 480);
    m_render_context->setObjectVector(&m_objects);
}


BaseApp::BaseApp(int gl_width, int gl_height){
    init(gl_width, gl_height);
    m_render_context->setObjectVector(&m_objects);
}


BaseApp::~BaseApp(){
}


void BaseApp::init(int gl_width, int gl_height){
    m_input.reset(new Input());
    m_camera.reset(new Camera(math::vec3(-0.0f, 50.0f, 50.0f), 67.0f, (float)gl_width / (float)gl_height , 0.1f, 100000.0f, m_input.get()));
    m_camera->setSpeed(10.f);
    m_window_handler.reset(new WindowHandler(gl_width, gl_height, m_input.get(), m_camera.get()));
    m_camera->setWindowHandler(m_window_handler.get());
    m_frustum.reset(new Frustum());
    m_render_context.reset(new RenderContext(m_camera.get(), m_window_handler.get(), &m_buffers));
    m_bt_wrapper.reset(new BtWrapper(btVector3(0, -9.81, 0), &m_buffers));

    m_buffers.last_updated = none;
}


void BaseApp::run(){
    while (!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        m_render_context->render(true);
        glfwSwapBuffers(m_window_handler->getWindow());
    }
    m_window_handler->terminate();
}

