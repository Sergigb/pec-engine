#ifndef INPUT_HPP
#define INPUT_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <vector>

#include "common.hpp"

#define INPUT_MBUTTON_UP        0x00
#define INPUT_MBUTTON_PRESS     0x01
#define INPUT_MBUTTON_REPEAT    0x02
#define INPUT_MBUTTON_RELEASE   0x04

#define INPUT_KEY_UP        0x00
#define INPUT_KEY_DOWN      0x01
#define INPUT_KEY_REPEAT    0x02
#define INPUT_KEY_RELEASE   0x04


class Input{
    private:
        int m_num_pressed_mbuttons;
        double m_mouse_posx, m_mouse_posy;
        double m_mouse_posx_prev, m_mouse_posy_prev;
        bool m_mouse_moved = false;
        std::vector<int> m_keys_pressed;
    public:
        char pressed_keys[GLFW_KEY_LAST + 1];
        char pressed_mbuttons[GLFW_MOUSE_BUTTON_LAST + 1];

        Input();
        //Input();
        ~Input(); 

        void update();
        
        bool keyboardPressed() const;
        bool mButtonPressed() const;
        void getMousePos(double& posx, double& posy) const;
        void getMousePosPrev(double& posx, double& posy) const;
        bool mouseMoved() const;

        void onKeyboardInput(int key, int scancode, int action, int mods);
        void onMouseButton(int button, int action, int mods);
        void onMousePos(double posx, double posy);
};


#endif
