#include "Input.hpp"


Input::Input(){
    std::memset(pressed_keys, 0, (GLFW_KEY_LAST + 1));
    std::memset(pressed_mbuttons, 0, (GLFW_MOUSE_BUTTON_LAST + 1));
    m_mouse_posx = 0;
    m_mouse_posy = 0;
    m_mouse_posx_prev = 0;
    m_mouse_posy_prev = 0;
    m_mouse_moved = false;
    m_xoffset = 0;
    m_yoffset = 0;
}


Input::~Input(){}


void Input::onKeyboardInput(int key, int scancode, int action, int mods){
    UNUSED(scancode);
    UNUSED(mods);

    if(key == GLFW_KEY_UNKNOWN) return;
    if(action == GLFW_PRESS){
        pressed_keys[key] = INPUT_KEY_DOWN;
        m_keys_pressed.push_back(key);
    }
    else if(action == GLFW_RELEASE){
        pressed_keys[key] = INPUT_KEY_RELEASE;
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


void Input::onMousePos(double posx, double posy){
    m_mouse_moved = true;
    m_mouse_posx = posx;
    m_mouse_posy = posy;
}


void Input::onScroll(double xoffset, double yoffset){
    m_xoffset = xoffset;
    m_yoffset = yoffset;
}


void Input::update(){
    // This function is used to update the status of the keyboard/mouse before calling the poll
    // events function. It deals mostly with the key/mbutton down/repeat/release
    m_mouse_moved = false;

    m_mouse_posx_prev = m_mouse_posx;
    m_mouse_posy_prev = m_mouse_posy;

    m_xoffset = 0;
    m_yoffset = 0;

    // update mbutton repeat/release
    for(int i=0; i < GLFW_MOUSE_BUTTON_LAST + 1; i++){
        if(pressed_mbuttons[i] == INPUT_MBUTTON_PRESS)
            pressed_mbuttons[i] = INPUT_MBUTTON_REPEAT;
        else if(pressed_mbuttons[i] == INPUT_MBUTTON_RELEASE)
            pressed_mbuttons[i] = 0;
    }

    std::vector<int>::iterator it = m_keys_pressed.begin();

    while(it != m_keys_pressed.end()){
        int key = *it;
        if(pressed_keys[key] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
            pressed_keys[key] = INPUT_KEY_REPEAT;
            ++it;
        }
        else if(pressed_keys[key] & INPUT_KEY_RELEASE){
            pressed_keys[key] = 0;
            it = m_keys_pressed.erase(it);
        }

    }
}


bool Input::keyboardPressed() const{
    return (m_keys_pressed.size() > 0);
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


void Input::getMousePosPrev(double& posx, double& posy) const{
    posx = m_mouse_posx_prev;
    posy = m_mouse_posy_prev;
}


void Input::getScroll(double& xoffset, double& yoffset) const{
    xoffset = m_xoffset;
    yoffset = m_yoffset;
}

