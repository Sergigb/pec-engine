#include "Input.hpp"


Input::Input(){
    std::memset(pressed_keys, 0, GLFW_KEY_LAST);
    m_num_pressed_keys = 0;
}


Input::~Input(){}


void Input::onKeyboardInput(int key, int scancode, int action, int mods){
    UNUSED(scancode);
    UNUSED(mods);

    if(key == GLFW_KEY_UNKNOWN) return;
    //key_pressed = true;
    if(action == GLFW_PRESS){
        pressed_keys[key] = true;
        m_num_pressed_keys += 1;
    }
    else if(action == GLFW_RELEASE){
        pressed_keys[key] = false;
        m_num_pressed_keys -= 1;
    }
}


void Input::update(){
    //key_pressed = false;
}


bool Input::keyboardPressed() const{
    return (m_num_pressed_keys > 0);
}

