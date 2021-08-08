#ifndef INPUT_HPP
#define INPUT_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>


#define INPUT_MBUTTON_UP        0x00
#define INPUT_MBUTTON_PRESS     0x01
#define INPUT_MBUTTON_REPEAT    0x02
#define INPUT_MBUTTON_RELEASE   0x04

#define INPUT_KEY_UP        0x01
#define INPUT_KEY_DOWN      0x02
#define INPUT_KEY_REPEAT    0x04
#define INPUT_KEY_RELEASE   0x08


/*
 * Class used to manage the input as an interface to glfw. 
 */

class Input{
    private:
        int m_num_pressed_mbuttons;
        double m_mouse_posx, m_mouse_posy;
        double m_mouse_posx_prev, m_mouse_posy_prev;
        double m_xoffset, m_yoffset;
        bool m_mouse_moved = false;
        std::vector<int> m_keys_pressed;
    public:
        /*
         * Status of the keyboard and mouse buttons, uses the glfw key values.
         * https://www.glfw.org/docs/latest/group__keys.html
         * https://www.glfw.org/docs/latest/group__buttons.html
         */
        char pressed_keys[GLFW_KEY_LAST + 1];
        char pressed_mbuttons[GLFW_MOUSE_BUTTON_LAST + 1];

        Input();
        ~Input(); 

        /*
         * This method is mostly used to prepare the mouse/keyboard status (key/mbutton down/
         * repeat/release) before the glfwPollEvents function in the windows handler is called.
         */
        void update();

        /*
         * These functions return true if the keyboard or the mouse buttons have been pressed.
         * mouseMoved returns true if the mouse moved.
         */        
        bool keyboardPressed() const;
        bool mButtonPressed() const;
        bool mouseMoved() const;

        /*
         * Returns by reference the mouse pos x and y in glfw coordinates. getMousePosPrev returns
         * the coordinates of the mouse during in previous tick.
         */
        void getMousePos(double& posx, double& posy) const;
        void getMousePosPrev(double& posx, double& posy) const;

        /*
         * Returns by reference the scroll of the mouse.
         */
        void getScroll(double& xoffset, double& yoffset) const;

        /*
         * These functions are called by the windows handler's input callback functions.
         */
        void onKeyboardInput(int key, int scancode, int action, int mods);
        void onMouseButton(int button, int action, int mods);
        void onMousePos(double posx, double posy);
        void onScroll(double xoffset, double yoffset);
};


#endif
