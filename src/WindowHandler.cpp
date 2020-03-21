#include "WindowHandler.hpp"



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


void glfw_error_callback(int error, const char* description){
    std::cout << "GLFW error " << error << "(" << description << ")" << std::endl;
    log("GLFW error number ", error, " (", description, ")");
}


void WindowHandler::initGlfw(){
    log("Starting GLFW ", glfwGetVersionString());

    glfwSetErrorCallback(glfw_error_callback);
    if(!glfwInit()){
        std::cerr << "ERROR: could not start GLFW3, check the log" << std::endl;
        log("ERROR: could not start GLFW3");
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

    m_window = glfwCreateWindow(m_gl_width, m_gl_height, "The window with no name", NULL, NULL);
    if(!m_window){
        std::cerr << "ERROR: could not open window with GLFW3, check the log" << std::endl;
        log("ERROR: could not open window with GLFW3");
        glfwTerminate();
    }

    glfwMakeContextCurrent(m_window);

    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, WindowHandler::glfwKeyCallback);
    glfwSetFramebufferSizeCallback(m_window, WindowHandler::glfwFramebufferSizeCallback);
    glfwSetMouseButtonCallback(m_window, WindowHandler::glfwMouseButtonCallback);
    glfwSetCursorPosCallback(m_window, WindowHandler::glfwMousePosCallback);
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


void WindowHandler::handleMouseButton(int button, int action, int mods){
    m_input->onMouseButton(button, action, mods);
}


void WindowHandler::handleMousePos(double posx, double posy){
    m_input->onMouseButtonPos(posx, posy);
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

void WindowHandler::glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods){
    WindowHandler* wh = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
    wh->handleMouseButton(button, action, mods);
}

void WindowHandler::glfwMousePosCallback(GLFWwindow *window, double posx, double posy){
    WindowHandler* wh = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
    wh->handleMousePos(posx, posy);
}