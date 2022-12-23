#include <iostream>

#include "WindowHandler.hpp"
#include "RenderContext.hpp"
#include "log.hpp"
#include "Input.hpp"
#include "Camera.hpp"


WindowHandler::WindowHandler(){
    m_window = nullptr;
    m_gl_width = 640;
    m_gl_height = 480;
    m_input = nullptr;
    m_camera = nullptr;
}


WindowHandler::WindowHandler(int width, int height, Input* input, Camera* camera){
    //m_window = window;
    m_gl_width = width;
    m_gl_height = height;
    m_input = input;
    m_camera = camera;

    initGlfw();
}


WindowHandler::~WindowHandler(){
}


void WindowHandler::setRenderContext(RenderContext* render_context){
    m_render_context = render_context;
}


void glfw_error_callback(int error, const char* description){
    std::cout << "glfw_error_callback: GLFW error " << error << "(" 
              << description << ")" << std::endl;
    log("glfw_error_callback: GLFW error number ", error, " (", description, ")");
}


void WindowHandler::initGlfw(){
    log("WindowHandler::initGlfw: Starting GLFW ", glfwGetVersionString());

    glfwSetErrorCallback(glfw_error_callback);
    if(!glfwInit()){
        std::cerr << "WindowHandler::initGlfw: could not start GLFW3, check the log" 
                  << std::endl;
        log("WindowHandler::initGlfw: could not start GLFW3");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    //glfwWindowHint(GLFW_SAMPLES, 4); //x4 MSAA, we should add a function to change it

//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /*GLFWmonitor* mon = glfwGetPrimaryMonitor ();
    const GLFWvidmode* vmode = glfwGetVideoMode (mon);
    m_window = glfwCreateWindow (
                    vmode->width, vmode->height, "Extended GL Init", mon, NULL
);*/

    m_window = glfwCreateWindow(m_gl_width, m_gl_height, "pec-engine", NULL, NULL);
    if(!m_window){
        std::cerr << "WindowHandler::initGlfw: could not open window with GLFW3, check the log"
                  << std::endl;
        log("WindowHandler::initGlfw: could not open window with GLFW3");
        glfwTerminate();
    }

    glfwMakeContextCurrent(m_window);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, WindowHandler::glfwKeyCallback);
    glfwSetFramebufferSizeCallback(m_window, WindowHandler::glfwFramebufferSizeCallback);
    glfwSetMouseButtonCallback(m_window, WindowHandler::glfwMouseButtonCallback);
    glfwSetCursorPosCallback(m_window, WindowHandler::glfwMousePosCallback);
    glfwSetScrollCallback(m_window, WindowHandler::glfwScrollCallback);
}


void WindowHandler::handleKeyboardInput(int key, int scancode, int action, int mods){
    m_input->onKeyboardInput(key, scancode, action, mods);
}


void WindowHandler::handleFramebufferSizeUpdate(int width, int height){
    m_gl_width = width;
    m_gl_height = height;
    log("WindowHandler::handleFramebufferSizeUpdate: screen resolution set to ",
        width, 'x', height);
    m_render_context->onFramebufferSizeUpdate(m_gl_width, m_gl_height);
    m_camera->onFramebufferSizeUpdate(m_gl_width, m_gl_height);
}


void WindowHandler::handleMouseButton(int button, int action, int mods){
    m_input->onMouseButton(button, action, mods);
}


void WindowHandler::handleMousePos(double posx, double posy){
    m_input->onMousePos(posx, posy);
}


void WindowHandler::handleScrollCallback(double xoffset, double yoffset){
    m_input->onScroll(xoffset, yoffset);
}


void WindowHandler::getFramebufferSize(int& gl_width, int& gl_heigth) const{
    gl_heigth = m_gl_height;
    gl_width = m_gl_width;
}


GLFWwindow* WindowHandler::getWindow() const{
    return m_window;
}


void WindowHandler::update(){
    glfwPollEvents();
}


void WindowHandler::setWindowShouldClose(){
    glfwSetWindowShouldClose(m_window, 1);
}


void WindowHandler::setWindowTitle(const char* title){
    glfwSetWindowTitle(m_window, title);
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

void WindowHandler::glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods){
    WindowHandler* wh = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
    wh->handleMouseButton(button, action, mods);
}

void WindowHandler::glfwMousePosCallback(GLFWwindow *window, double posx, double posy){
    WindowHandler* wh = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
    wh->handleMousePos(posx, posy);
}


void WindowHandler::glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset){
    WindowHandler* wh = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
    wh->handleScrollCallback(xoffset, yoffset);
}


void WindowHandler::terminate(){
    log("WindowHandler::terminate: terminating GLFW and exiting");
    std::cout << "WindowHandler::terminate: terminating GLFW and exiting" << std::endl;
    glfwTerminate();
}


