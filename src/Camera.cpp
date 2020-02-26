#include "Camera.hpp"


Camera::Camera(){
    m_cam_translation = vec3(0.0f, 0.0f, 0.0f);
    m_fwd = vec4(0.0f, 0.0f, -1.0f, 0.0f);
    m_rgt = vec4(1.0f, 0.0f, 0.0f, 0.0f);
    m_up = vec4(0.0f, 1.0f, 0.0f, 0.0f);
    m_cam_pos = vec3(0.f, 0.f, 5.0f);
    m_cam_orientation = quat_from_axis_rad(0.0f, 0.0f, 1.0f, 0.0f);
    m_proj_mat = perspective(67.0f, 1.0, 0.1f, 1000.0f);
    input = nullptr;
    m_cam_speed = 3.5f;
    m_cam_heading_speed = 3.5f;
    m_previous_frame_time = glfwGetTime();
    m_near = 0.1f;
    m_far = 100.f;
    m_fovy = 67.f;
    m_ar = 1.f;
    m_has_moved = false;
    m_proj_change = false;
    m_fb_callback = false;
    m_mouse_posx_last = 0;
    m_mouse_posy_last = 0;
    m_cam_input_mode = GLFW_CURSOR_NORMAL;
    updateViewMatrix();
}


Camera::Camera(const vec3* pos, float fovy, float ar, float near, float far, const Input* input_ptr){
    m_cam_pos = *pos;
    m_cam_translation = vec3(0.0f, 0.0f, 0.0f);
    m_fwd = vec4(0.0f, 0.0f, -1.0f, 0.0f);
    m_rgt = vec4(1.0f, 0.0f, 0.0f, 0.0f);
    m_up = vec4(0.0f, 1.0f, 0.0f, 0.0f);
    m_cam_orientation = quat_from_axis_rad(0.0f, 0.0f, 0.0f, 0.0f);
    m_proj_mat = perspective(fovy, ar, near, far);
    input = input_ptr;
    m_cam_speed = 3.5f;
    m_cam_heading_speed = 3.5f;
    m_previous_frame_time = glfwGetTime();
    m_near = near;
    m_far = far;
    m_fovy = fovy;
    m_ar = ar;
    m_has_moved = false;
    m_proj_change = false;
    m_fb_callback = false;
    m_mouse_posx_last = 320;
    m_mouse_posy_last = 240;
    m_cam_input_mode = GLFW_CURSOR_NORMAL;
    updateViewMatrix();
}

Camera::~Camera(){
    
}


void Camera::setCameraOrientation(const versor* orientation){
    m_cam_orientation = *orientation; 
}


void Camera::setCameraOrientationFromAxisRad(float cam_heading, const vec3* axis){
    m_cam_orientation = quat_from_axis_rad(-cam_heading, axis->v[0], axis->v[1], axis->v[2]);    
}


void Camera::setCameraOrientationFromAxisDeg(float cam_heading, const vec3* axis){
    m_cam_orientation = quat_from_axis_deg(-cam_heading, axis->v[0], axis->v[1], axis->v[2]);    
}


void Camera::createProjMat(float near, float far, float fovy, float ar){
    m_near = near;
    m_far = far;
    m_fovy = fovy;
    m_ar = ar;
    m_proj_mat = perspective(m_fovy, m_ar, m_near, m_far);
}

void Camera::onFramebufferSizeUpdate(int width, int heigth){
    m_proj_mat = perspective(m_fovy, (float)width/(float)heigth, m_near, m_far);
    m_fb_callback = true;
}


mat4 Camera::getViewMatrix() const{
    return m_view_matrix;
}


mat4 Camera::getProjMatrix() const{
    return m_proj_mat;
}


// TODO: maybe put all these three functions in one function?
void Camera::rotateCameraYaw(float degrees){
    mat4 R;
    versor q_yaw = quat_from_axis_rad(degrees, 0.0f, 1.0f, 0.0f); // rotate around the y vector to keep the right vector parallel to the "ground"

    m_cam_orientation = q_yaw * m_cam_orientation;
    R = quat_to_mat4(m_cam_orientation);
    m_fwd = R * vec4(0.0, 0.0, -1.0, 0.0);
    m_rgt = R * vec4(1.0, 0.0, 0.0, 0.0);
    m_up = R * vec4(0.0, 1.0, 0.0, 0.0);
}


void Camera::rotateCameraRoll(float degrees){
    mat4 R;
    versor q_pitch = quat_from_axis_rad(degrees, m_fwd.v[0], m_fwd.v[1], m_fwd.v[2]);

    m_cam_orientation = q_pitch * m_cam_orientation;
    R = quat_to_mat4(m_cam_orientation);
    m_fwd = R * vec4(0.0, 0.0, -1.0, 0.0);
    m_rgt = R * vec4(1.0, 0.0, 0.0, 0.0);
    m_up = R * vec4(0.0, 1.0, 0.0, 0.0);
}


void Camera::rotateCameraPitch(float degrees){
    mat4 R;
    versor q_roll = quat_from_axis_rad(degrees, m_rgt.v[0], m_rgt.v[1], m_rgt.v[2]);

    m_cam_orientation = q_roll * m_cam_orientation;
    R = quat_to_mat4(m_cam_orientation);
    m_fwd = R * vec4(0.0, 0.0, -1.0, 0.0);
    m_rgt = R * vec4(1.0, 0.0, 0.0, 0.0);
    m_up = R * vec4(0.0, 1.0, 0.0, 0.0);
}


void Camera::moveCamera(const vec3* motion){
    m_cam_translation += *motion;
}

void Camera::updateViewMatrix(){
    mat4 R, T;
    
    m_cam_pos = m_cam_pos + vec3(m_fwd) * -m_cam_translation.v[2];
    m_cam_pos = m_cam_pos + vec3(m_up) * m_cam_translation.v[1];
    m_cam_pos = m_cam_pos + vec3(m_rgt) * m_cam_translation.v[0];
    T = translate(identity_mat4(), vec3(m_cam_pos));
    R = quat_to_mat4(m_cam_orientation);

    m_cam_translation = vec3(0.0f, 0.0f, 0.0f);

    m_view_matrix = inverse(R) * inverse(T);
}


void Camera::update(){
    double elapsed_time, current_frame_time = glfwGetTime();
    float cam_yaw = 0.0f, cam_pitch = 0.0f, cam_roll = 0.0f;
    mat4 R, T;

    elapsed_time = current_frame_time - m_previous_frame_time;
    m_previous_frame_time = current_frame_time;

    if(m_fb_callback){
        m_proj_change = true;
        m_fb_callback = false;
    }
    else if(m_proj_change)
        m_proj_change = false;
    

    if(!input->keyboardPressed() && !input->mouseMoved()){
        m_has_moved = false;
        return;
    }

    // we should be able to reduce the number of comparisons and other stuff
    if(input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2] && m_cam_input_mode == GLFW_CURSOR_NORMAL){
        glfwSetInputMode(m_g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_cam_input_mode = GLFW_CURSOR_DISABLED;
        input->getMousePos(m_mouse_posx_last, m_mouse_posy_last);
    }
    else if(!input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2] && m_cam_input_mode == GLFW_CURSOR_DISABLED){
        glfwSetInputMode(m_g_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_cam_input_mode = GLFW_CURSOR_NORMAL;
    }

    if(input->mouseMoved() && input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2]){
        double dif_x, dif_y, posx, posy;
        input->getMousePos(posx, posy);
        dif_x = m_mouse_posx_last - posx;
        dif_y = m_mouse_posy_last - posy;
        
        cam_pitch += dif_y * 1 * elapsed_time;
        cam_yaw += dif_x * 1 * elapsed_time;

        rotateCameraPitch(cam_pitch);
        rotateCameraYaw(cam_yaw);

        m_mouse_posx_last = posx;
        m_mouse_posy_last = posy;
    }

    if(input->pressed_keys[GLFW_KEY_A]){
        m_cam_translation.v[0] -= m_cam_speed * elapsed_time;
    }
    if(input->pressed_keys[GLFW_KEY_D]){
        m_cam_translation.v[0] += m_cam_speed * elapsed_time;
    }
    if(input->pressed_keys[GLFW_KEY_Z]){
        m_cam_translation.v[1] -= m_cam_speed * elapsed_time;
    }
    if(input->pressed_keys[GLFW_KEY_C]){
        m_cam_translation.v[1] += m_cam_speed * elapsed_time;
    }
    if(input->pressed_keys[GLFW_KEY_W]){
        m_cam_translation.v[2] -= m_cam_speed * elapsed_time;
    }
    if(input->pressed_keys[GLFW_KEY_S]){
        m_cam_translation.v[2] += m_cam_speed * elapsed_time;
    }
    if(input->pressed_keys[GLFW_KEY_LEFT]){
        cam_yaw += m_cam_heading_speed * elapsed_time;
        rotateCameraYaw(cam_yaw);
    }
    if(input->pressed_keys[GLFW_KEY_RIGHT]){
        cam_yaw -= m_cam_heading_speed * elapsed_time;
        rotateCameraYaw(cam_yaw);
    }
    if(input->pressed_keys[GLFW_KEY_UP]){
        cam_pitch += m_cam_heading_speed * elapsed_time;
        rotateCameraPitch(cam_pitch);
    }
    if(input->pressed_keys[GLFW_KEY_DOWN]){
        cam_pitch -= m_cam_heading_speed * elapsed_time;
        rotateCameraPitch(cam_pitch);
    }
    if(input->pressed_keys[GLFW_KEY_Q]){
        cam_roll -= m_cam_heading_speed * elapsed_time;
        rotateCameraRoll(cam_roll);
    }
    if(input->pressed_keys[GLFW_KEY_E]){
        cam_roll += m_cam_heading_speed * elapsed_time;
        rotateCameraRoll(cam_roll);
    }
    m_has_moved = true;
    updateViewMatrix();
}


void Camera::setSpeed(float speed){
    m_cam_speed = speed;
}


void Camera::setAngularSpeed(float speed){
    m_cam_heading_speed = speed;
}


bool Camera::hasMoved() const{
    return m_has_moved;
}


bool Camera::projChanged() const{
    return m_proj_change;
}


vec3 Camera::getCamPosition() const{
    return m_cam_pos;
}


void Camera::setWindow(GLFWwindow* g_window){
    m_g_window = g_window;
}

