#ifndef CAMERA_HPP
#define CAMERA_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

#include "maths_funcs.hpp"
#include "common.hpp"
#include "Input.hpp"
#include "log.hpp"

using namespace math;


class WindowHandler;


class Camera{
    private:
        vec3 m_cam_pos, m_cam_translation;
        vec4 m_fwd, m_rgt, m_up;
        versor m_cam_orientation;
        mat4 m_proj_mat, m_view_matrix;
        float m_cam_speed, m_cam_heading_speed, m_near, m_far, m_fovy, m_ar;
        double m_previous_frame_time;
        bool m_has_moved, m_proj_change, m_fb_callback;
        int m_cam_input_mode;

        const WindowHandler* m_window_handler;
        const Input* m_input;

        void updateViewMatrix();
    public:
        Camera();
        Camera(const vec3& pos, float fovy, float ar, float near, float far, const Input* input_ptr);
        ~Camera(); 

        void setCameraOrientation(const versor* orientation);
        void setCameraOrientationFromAxisRad(float cam_heading, const vec3* axis);
        void setCameraOrientationFromAxisDeg(float cam_heading, const vec3* axis);
        void setSpeed(float speed);
        void setAngularSpeed(float speed);
        void setWindowHandler(const WindowHandler* window_handler);
        void createProjMat(float near, float far, float fovy, float ar);
        void onFramebufferSizeUpdate(int width, int heigth);

        void rotateCameraYaw(float degrees);
        void rotateCameraRoll(float degrees);
        void rotateCameraPitch(float degrees);
        void moveCamera(const vec3* motion);

        bool hasMoved() const;
        bool projChanged() const;
        mat4 getViewMatrix() const;
        mat4 getProjMatrix() const;
        vec3 getCamPosition() const;
        void castRayMousePos(float dist, math::vec3& ray_start_world, math::vec3& ray_end_world_ext) const;

        void update();
};


#endif
