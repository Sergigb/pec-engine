#ifndef WINDOW_HANDLER_HPP
#define WINDOW_HANDLER_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "common.hpp"
#include "Input.hpp"
#include "Camera.hpp"
#include "log.hpp"


class WindowHandler{
    private:
        GLFWwindow* m_window;
        int m_gl_width, m_gl_height;
        Input* m_input;
        Camera* m_camera;

        void initGlfw();
    public:
        WindowHandler();
        WindowHandler(int width, int height, Input* input, Camera* camera);
        ~WindowHandler();

        void handleKeyboardInput(int key, int scancode, int action, int mods);
        void handleFramebufferSizeUpdate(int width, int height);
        void handleMouseButton(int button, int action, int mods);
        void handleMousePos(double posx, double posy);

        void getFramebufferSize(int& gl_width, int& gl_heigth) const;
        GLFWwindow* getWindow() const;

        static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void glfwFramebufferSizeCallback(GLFWwindow *window, int width, int height);
        static void glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        static void glfwMousePosCallback(GLFWwindow *window, double posx, double posy);

        void update();
};


#endif
