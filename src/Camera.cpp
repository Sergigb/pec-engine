#include "Camera.hpp"
#include "WindowHandler.hpp"


Camera::Camera(){
    m_cam_translation = dmath::vec3(0.0f, 0.0f, 0.0f);
    m_fwd = dmath::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    m_rgt = dmath::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    m_up = dmath::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    m_cam_pos = dmath::vec3(0.f, 0.f, 5.0f);
    m_cam_orientation = dmath::quat_from_axis_rad(0.0f, 0.0f, 1.0f, 0.0f);
    m_proj_mat = math::perspective(67.0f, 1.0, 0.1f, 1000.0f);
    m_input = nullptr;
    m_cam_speed = 3.5f;
    m_cam_heading_speed = 3.5f;
    m_previous_frame_time = glfwGetTime();
    m_near = 0.1f;
    m_far = 100.f;
    m_fovy = 67.f;
    m_ar = 1.f;
    m_proj_change = false;
    m_fb_callback = false;
    m_cam_input_mode = GLFW_CURSOR_NORMAL;
    updateViewMatrix();
    m_elapsed_time = 0;
}


Camera::Camera(const dmath::vec3& pos, float fovy, float ar, float near, float far, const Input* input_ptr){
    m_cam_pos = pos;
    m_cam_translation = dmath::vec3(0.0f, 0.0f, 0.0f);
    m_fwd = dmath::vec4(0.0f, 0.0f, -1.0f, 0.0f);
    m_rgt = dmath::vec4(1.0f, 0.0f, 0.0f, 0.0f);
    m_up = dmath::vec4(0.0f, 1.0f, 0.0f, 0.0f);
    m_cam_orientation = dmath::quat_from_axis_rad(0.0f, 0.0f, 0.0f, 0.0f);
    m_proj_mat = math::perspective(fovy, ar, near, far);
    m_input = input_ptr;
    m_cam_speed = 3.5f;
    m_cam_heading_speed = 3.5f;
    m_previous_frame_time = glfwGetTime();
    m_near = near;
    m_far = far;
    m_fovy = fovy;
    m_ar = ar;
    m_proj_change = false;
    m_fb_callback = false;
    m_cam_input_mode = GLFW_CURSOR_NORMAL;
    updateViewMatrix();
    m_elapsed_time = 0;
}

Camera::~Camera(){
    
}


void Camera::setCameraOrientation(const dmath::versor* orientation){
    m_cam_orientation = *orientation; 
}


void Camera::setCameraOrientationFromAxisRad(float cam_heading, const dmath::vec3* axis){
    m_cam_orientation = dmath::quat_from_axis_rad(-cam_heading, axis->v[0], axis->v[1], axis->v[2]);    
}


void Camera::createProjMat(float near, float far, float fovy, float ar){
    m_near = near;
    m_far = far;
    m_fovy = fovy;
    m_ar = ar;
    m_proj_mat = math::perspective(m_fovy, m_ar, m_near, m_far);
}

void Camera::onFramebufferSizeUpdate(int width, int height){
    m_proj_mat = math::perspective(m_fovy, (float)width/(float)height, m_near, m_far);
    m_fb_callback = true;
}


math::mat4 Camera::getViewMatrix() const{
    /* This should be precomputed, doesn't matter right now because we still have the single precision problem*/
    math::mat4 m(m_view_matrix.m[0], m_view_matrix.m[1], m_view_matrix.m[2], m_view_matrix.m[3], 
                 m_view_matrix.m[4], m_view_matrix.m[5], m_view_matrix.m[6], m_view_matrix.m[7], 
                 m_view_matrix.m[8], m_view_matrix.m[9], m_view_matrix.m[10], m_view_matrix.m[11], 
                 m_view_matrix.m[12], m_view_matrix.m[13], m_view_matrix.m[14], m_view_matrix.m[15]);
    return m;
}


math::mat4 Camera::getCenteredViewMatrix() const{
    math::mat4 m(m_view_matrix.m[0], m_view_matrix.m[1], m_view_matrix.m[2], m_view_matrix.m[3], 
                 m_view_matrix.m[4], m_view_matrix.m[5], m_view_matrix.m[6], m_view_matrix.m[7], 
                 m_view_matrix.m[8], m_view_matrix.m[9], m_view_matrix.m[10], m_view_matrix.m[11], 
                 0.0, 0.0, 0.0, m_view_matrix.m[15]);

    return m;
}


math::mat4 Camera::getProjMatrix() const{
    return m_proj_mat;
}


// TODO: maybe put all these three functions in one function?
void Camera::rotateCameraYaw(double degrees){
    dmath::mat4 R;
    dmath::versor q_yaw = dmath::quat_from_axis_rad(degrees, 0.0f, 1.0f, 0.0f); // rotate around the y vector to keep the right vector parallel to the "ground"

    m_cam_orientation = q_yaw * m_cam_orientation;
    R = dmath::quat_to_mat4(m_cam_orientation);
    m_fwd = R * dmath::vec4(0.0, 0.0, -1.0, 0.0);
    m_rgt = R * dmath::vec4(1.0, 0.0, 0.0, 0.0);
    m_up = R * dmath::vec4(0.0, 1.0, 0.0, 0.0);
}


void Camera::rotateCameraRoll(double degrees){
    dmath::mat4 R;
    dmath::versor q_pitch = dmath::quat_from_axis_rad(degrees, m_fwd.v[0], m_fwd.v[1], m_fwd.v[2]);

    m_cam_orientation = q_pitch * m_cam_orientation;
    R = dmath::quat_to_mat4(m_cam_orientation);
    m_fwd = R * dmath::vec4(0.0, 0.0, -1.0, 0.0);
    m_rgt = R * dmath::vec4(1.0, 0.0, 0.0, 0.0);
    m_up = R * dmath::vec4(0.0, 1.0, 0.0, 0.0);
}


void Camera::rotateCameraPitch(double degrees){
    dmath::mat4 R;
    dmath::versor q_roll = dmath::quat_from_axis_rad(degrees, m_rgt.v[0], m_rgt.v[1], m_rgt.v[2]);

    m_cam_orientation = q_roll * m_cam_orientation;
    R = dmath::quat_to_mat4(m_cam_orientation);
    m_fwd = R * dmath::vec4(0.0, 0.0, -1.0, 0.0);
    m_rgt = R * dmath::vec4(1.0, 0.0, 0.0, 0.0);
    m_up = R * dmath::vec4(0.0, 1.0, 0.0, 0.0);
}


void Camera::moveCamera(const dmath::vec3* motion){
    m_cam_translation += *motion;
}

void Camera::updateViewMatrix(){
    dmath::mat4 R, T;
    
    m_cam_pos = m_cam_pos + dmath::vec3(m_fwd) * -m_cam_translation.v[2];
    m_cam_pos = m_cam_pos + dmath::vec3(m_up) * m_cam_translation.v[1];
    m_cam_pos = m_cam_pos + dmath::vec3(m_rgt) * m_cam_translation.v[0];
    T = dmath::translate(dmath::identity_mat4(), dmath::vec3(m_cam_pos));
    R = dmath::quat_to_mat4(m_cam_orientation);

    m_cam_translation = dmath::vec3(0.0f, 0.0f, 0.0f);

    m_view_matrix = dmath::inverse(R) * dmath::inverse(T);
}


void Camera::update(){
    double current_frame_time = glfwGetTime();

    m_elapsed_time = current_frame_time - m_previous_frame_time;
    m_previous_frame_time = current_frame_time;

    if(m_fb_callback){
        m_proj_change = true;
        m_fb_callback = false;
    }
    else if(m_proj_change)
        m_proj_change = false;

    updateViewMatrix();
}


void Camera::setSpeed(float speed){
    m_cam_speed = speed;
}


void Camera::setAngularSpeed(float speed){
    m_cam_heading_speed = speed;
}


bool Camera::projChanged() const{
    return m_proj_change;
}


dmath::vec3 Camera::getCamPosition() const{
    return m_cam_pos;
}


void Camera::setWindowHandler(const WindowHandler* window_handler){
    m_window_handler = window_handler;
}


void Camera::castRayMousePos(float dist, dmath::vec3& ray_start_world, dmath::vec3& ray_end_world_ext) const{
    // ray_end_world_ext is ray_start_world + ray_direction * dist
    dmath::vec4 ray_start, ray_end, ray_end_world, ray_start_world_vec4;
    dmath::mat4 M, d_proj_mat;
    dmath::vec3 ray_dir;
    double mouse_x, mouse_y;
    int fb_width, fb_height;

    d_proj_mat = dmath::mat4(m_proj_mat.m[0], m_proj_mat.m[1], m_proj_mat.m[2], m_proj_mat.m[3], 
                             m_proj_mat.m[4], m_proj_mat.m[5], m_proj_mat.m[6], m_proj_mat.m[7], 
                             m_proj_mat.m[8], m_proj_mat.m[9], m_proj_mat.m[10], m_proj_mat.m[11], 
                             m_proj_mat.m[12], m_proj_mat.m[13], m_proj_mat.m[14], m_proj_mat.m[15]);

    m_window_handler->getFramebufferSize(fb_width, fb_height);

    m_input->getMousePos(mouse_x, mouse_y);
    mouse_y = fb_height - mouse_y; // y is inverted
    
    ray_start = dmath::vec4(((float)mouse_x/fb_width - 0.5) * 2.0,
                            (mouse_y/(float)fb_height - 0.5) * 2.0,
                            -1.0, 1.0);
    ray_end = dmath::vec4(((float)mouse_x/fb_width - 0.5) * 2.0,
                          (mouse_y/(float)fb_height - 0.5) * 2.0,
                          0.0, 1.0);

    M = dmath::inverse(d_proj_mat * m_view_matrix);

    ray_start_world_vec4 = M * ray_start;
    ray_start_world_vec4 = ray_start_world_vec4 / ray_start_world_vec4.v[3];
    ray_end_world = M * ray_end;
    ray_end_world = ray_end_world / ray_end_world.v[3];

    ray_dir = dmath::normalise(ray_end_world - ray_start_world_vec4);

    ray_start_world = dmath::vec3(ray_start_world_vec4);
    ray_end_world_ext = ray_start_world + ray_dir * dist; // ray end extended according to dist (in meters)
}


void Camera::setForwardVector(const dmath::vec4 vec){
    m_fwd = vec;
}


void Camera::setRightVector(const dmath::vec4 vec){
    m_rgt = vec;
}


void Camera::setUpVector(const dmath::vec4 vec){
    m_up = vec;
}


void Camera::setCameraTranslation(const dmath::vec3& translation){
    m_cam_translation = translation;
}


void Camera::freeCameraUpdate(){
    float cam_yaw = 0.0f, cam_pitch = 0.0f, cam_roll = 0.0f, speed_mult = 1.0f;

    if(!m_input->keyboardPressed() && !m_input->mouseMoved()){
        return;
    }

    if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2] && m_cam_input_mode == GLFW_CURSOR_NORMAL){
        glfwSetInputMode(m_window_handler->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        m_cam_input_mode = GLFW_CURSOR_DISABLED;
    }
    else if(!m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2] && m_cam_input_mode == GLFW_CURSOR_DISABLED){
        glfwSetInputMode(m_window_handler->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        m_cam_input_mode = GLFW_CURSOR_NORMAL;
    }

    if(m_input->mouseMoved() && m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2]){
        double dif_x, dif_y, posx, posy, mouse_posx_last, mouse_posy_last;
        m_input->getMousePos(posx, posy);
        m_input->getMousePosPrev(mouse_posx_last, mouse_posy_last);
        dif_x = mouse_posx_last - posx;
        dif_y = mouse_posy_last - posy;
        
        cam_pitch += dif_y * 0.25 * m_elapsed_time;
        cam_yaw += dif_x * 0.25 * m_elapsed_time;

        rotateCameraPitch(cam_pitch);
        rotateCameraYaw(cam_yaw);
    }

    if(m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT])
        speed_mult = 4.0f;

    if(m_input->pressed_keys[GLFW_KEY_A] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        m_cam_translation.v[0] -= m_cam_speed * speed_mult * m_elapsed_time;
    }
    if(m_input->pressed_keys[GLFW_KEY_D] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        m_cam_translation.v[0] += m_cam_speed * speed_mult * m_elapsed_time;
    }
    if(m_input->pressed_keys[GLFW_KEY_Z] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        m_cam_translation.v[1] -= m_cam_speed * m_elapsed_time;
    }
    if(m_input->pressed_keys[GLFW_KEY_C] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        m_cam_translation.v[1] += m_cam_speed * m_elapsed_time;
    }
    if(m_input->pressed_keys[GLFW_KEY_W] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        m_cam_translation.v[2] -= m_cam_speed * speed_mult * m_elapsed_time;
    }
    if(m_input->pressed_keys[GLFW_KEY_S] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        m_cam_translation.v[2] += m_cam_speed * speed_mult * m_elapsed_time;
    }
    if(m_input->pressed_keys[GLFW_KEY_LEFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        cam_yaw += m_cam_heading_speed * m_elapsed_time;
        rotateCameraYaw(cam_yaw);
    }
    if(m_input->pressed_keys[GLFW_KEY_RIGHT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        cam_yaw -= m_cam_heading_speed * m_elapsed_time;
        rotateCameraYaw(cam_yaw);
    }
    if(m_input->pressed_keys[GLFW_KEY_UP] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        cam_pitch += m_cam_heading_speed * m_elapsed_time;
        rotateCameraPitch(cam_pitch);
    }
    if(m_input->pressed_keys[GLFW_KEY_DOWN] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        cam_pitch -= m_cam_heading_speed * m_elapsed_time;
        rotateCameraPitch(cam_pitch);
    }
    if(m_input->pressed_keys[GLFW_KEY_Q] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        cam_roll -= m_cam_heading_speed * m_elapsed_time;
        rotateCameraRoll(cam_roll);
    }
    if(m_input->pressed_keys[GLFW_KEY_E] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
        cam_roll += m_cam_heading_speed * m_elapsed_time;
        rotateCameraRoll(cam_roll);
    }
}

