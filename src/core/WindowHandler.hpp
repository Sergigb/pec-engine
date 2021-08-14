#ifndef WINDOW_HANDLER_HPP
#define WINDOW_HANDLER_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>


class RenderContext;
class Input;
class Camera;

/*
 * Manages the main window. If we want more windows (do we? maybe in the future?), we should make
 * a different class, or a base class... It essentially deals with GLFW callbacks and other 
 * shenanigans.
 */
class WindowHandler{
    private:
        GLFWwindow* m_window;
        int m_gl_width, m_gl_height;
        Input* m_input;
        Camera* m_camera;
        RenderContext* m_render_context;

        void initGlfw();
    public:
        WindowHandler();

        /*
         * Constructor.
         *
         * @width: initial width of the screen.
         * @height: initial height of the screen.
         * @input: pointer to the input object.
         * @camera: pointer to the camera object.
         */
        WindowHandler(int width, int height, Input* input, Camera* camera);
        ~WindowHandler();

        /*
         * Sets the render context.
         *
         * @render_context: pointer to the render context object.
         */
        void setRenderContext(RenderContext* render_context);

        /*
         * Functions called from the static methods below, they process the input.
         */
        void handleKeyboardInput(int key, int scancode, int action, int mods);
        void handleFramebufferSizeUpdate(int width, int height);
        void handleMouseButton(int button, int action, int mods);
        void handleMousePos(double posx, double posy);
        void handleScrollCallback(double xoffset, double yoffset);

        /*
         * Returns by reference the framebuffer size.
         *
         * @gl_width: width of the framebuffer.
         * @gl_heigth: heigth of the framebuffer.
         */
        void getFramebufferSize(int& gl_width, int& gl_heigth) const;

        /*
         * Returns a pointer to the GLFW window object.
         */
        GLFWwindow* getWindow() const;

        /*
         * Static methods called by GLFW, they deal with the input and other events. They call the
         * handling functions from above.
         */
        static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        static void glfwFramebufferSizeCallback(GLFWwindow *window, int width, int height);
        static void glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        static void glfwMousePosCallback(GLFWwindow *window, double posx, double posy);
        static void glfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

        /*
         * Update method to be called from the main thread, for now in polls GLFW events.
         */
        void update();

        /*
         * Terminates GLFW. NOT the current window, the whole thing.
         */
        void terminate();

        /*
         * Tells the window it should close, when the update method is called it will close the
         * window, and then the application should exit.
         */
        void setWindowShouldClose();

        /*
         * Sets the title of the window.
         *
         * @title: char string with the new name of the window, null-terminated.
         */
        void setWindowTitle(const char* title);
};


#endif
