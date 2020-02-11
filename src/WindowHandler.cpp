#include "WindowHandler.hpp"



WindowHandler::WindowHandler(){
    m_window = nullptr;
    m_gl_width = 640;
    m_gl_height = 480;
    m_input = nullptr;
    m_camera = nullptr;
}


WindowHandler::WindowHandler(GLFWwindow* window, int width, int height, Input* input, Camera* camera){
    m_window = window;
    m_gl_width = width;
    m_gl_height = height;
    m_input = input;
    m_camera = camera;
}


WindowHandler::~WindowHandler(){
}


void WindowHandler::handleKeyboardInput(int key, int scancode, int action, int mods){
    m_input->onKeyboardInput(key, scancode, action, mods);
}


void WindowHandler::handleFramebufferSizeUpdate(int width, int height){
    m_gl_width = width;
    m_gl_height = height;
    log("Screen resolution set to ", width, 'x', m_gl_height);
    glViewport(0, 0, m_gl_width, m_gl_height);
    m_camera->onFramebufferSizeUpdate(m_gl_width, m_gl_height);
}

void WindowHandler::getFramebufferSize(int& gl_width, int& gl_heigth) const{
    gl_heigth = m_gl_height;
    gl_width = m_gl_width;
}


GLFWwindow* WindowHandler::getWindow() const{
    return m_window;
}


void WindowHandler::update(){
    if(m_input->pressed_keys[GLFW_KEY_ESCAPE]){
        glfwSetWindowShouldClose(m_window, 1);
        return;
    }
}


/////// static functions ///////

void WindowHandler::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
    WindowHandler* wh = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
    wh->handleKeyboardInput(key, scancode, action, mods);
}

void WindowHandler::glfwFramebufferSizeCallback(GLFWwindow *window, int width, int height){
    WindowHandler* wh = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
    wh->handleFramebufferSizeUpdate(width, height);
}