#ifndef INPUT_HPP
#define INPUT_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>

#include "common.hpp"

//TODO: const correctness

class Input{
    private:
        int m_num_pressed_keys;
    public:
        bool pressed_keys[GLFW_KEY_LAST];

        Input();
        //Input();
        ~Input(); 

        void update();
        bool keyboardPressed() const;
        void onKeyboardInput(int key, int scancode, int action, int mods);
};


#endif
