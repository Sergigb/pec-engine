#include "Input.hpp"


Input::Input(){
    std::memset(pressed_keys, 0, GLFW_KEY_LAST + 1);
    std::memset(pressed_mbuttons, 0, GLFW_MOUSE_BUTTON_LAST + 1 * sizeof(int));
    m_num_pressed_keys = 0;
    m_mouse_posx = 0;
    m_mouse_posy = 0;
    m_mouse_moved = false;
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


void Input::onMouseButton(int button, int action, int mods){
    UNUSED(mods);

    if(action == GLFW_PRESS){
        pressed_mbuttons[button] = INPUT_MBUTTON_PRESS;
    }
    else if(action == GLFW_RELEASE){
        pressed_mbuttons[button] = INPUT_MBUTTON_RELEASE;
    }
}


void Input::onMouseButtonPos(double posx, double posy){
    m_mouse_moved = true;
    m_mouse_posx = posx;
    m_mouse_posy = posy;
}


void Input::update(){
    m_mouse_moved = false;

    // update mbutton repeat/release
    for(int i=0; i < GLFW_MOUSE_BUTTON_LAST + 1; i++){
        if(pressed_mbuttons[i] == INPUT_MBUTTON_PRESS)
            pressed_mbuttons[i] = INPUT_MBUTTON_REPEAT;
        else if(pressed_mbuttons[i] == INPUT_MBUTTON_RELEASE)
            pressed_mbuttons[i] = 0;
    }
}


bool Input::keyboardPressed() const{
    return (m_num_pressed_keys > 0);
}


bool Input::mButtonPressed() const{
    for(int i = 0; i < GLFW_MOUSE_BUTTON_LAST + 1; i++)
        if(pressed_mbuttons[i])
            return true;
    return false;
}


bool Input::mouseMoved() const{
    return m_mouse_moved;
}


void Input::getMousePos(double& posx, double& posy) const{
    posx = m_mouse_posx;
    posy = m_mouse_posy;
}

