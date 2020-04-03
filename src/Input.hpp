#ifndef INPUT_HPP
#define INPUT_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>

#include "common.hpp"


#define INPUT_MBUTTON_PRESS 1
#define INPUT_MBUTTON_REPEAT 2
#define INPUT_MBUTTON_RELEASE 3


class Input{
    private:
        int m_num_pressed_keys;
        int m_num_pressed_mbuttons;
        double m_mouse_posx, m_mouse_posy;
        bool m_mouse_moved = false;
    public:
        bool pressed_keys[GLFW_KEY_LAST + 1];
        int pressed_mbuttons[GLFW_MOUSE_BUTTON_LAST + 1];

        Input();
        //Input();
        ~Input(); 

        void update();
        
        bool keyboardPressed() const;
        bool mButtonPressed() const;
        void getMousePos(double& posx, double& posy) const;
        bool mouseMoved() const;

        void onKeyboardInput(int key, int scancode, int action, int mods);
        void onMouseButton(int button, int action, int mods);
        void onMouseButtonPos(double posx, double posy);
};


#endif
