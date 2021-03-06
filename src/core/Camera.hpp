#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "maths_funcs.hpp"


class WindowHandler;
class Input;


class Camera{
    private:
        dmath::vec3 m_cam_pos, m_cam_translation;
        dmath::vec4 m_fwd, m_rgt, m_up;
        dmath::versor m_cam_orientation;
        math::mat4 m_proj_mat;
        dmath::mat4 m_view_matrix;
        float m_cam_speed, m_cam_heading_speed, m_near, m_far, m_fovy, m_ar;
        double m_previous_frame_time, m_elapsed_time;
        bool m_proj_change, m_fb_callback;
        int m_cam_input_mode, m_prev_cam_input_mode;

        // oribital camera params
        double m_polar_angle, m_azimuthal_angle, m_radial_distance, m_inclination;
        dmath::vec3 m_inclination_axis;

        const WindowHandler* m_window_handler;
        const Input* m_input;

        void updateViewMatrix();
    public:
        Camera();
        Camera(const dmath::vec3& pos, float fovy, float ar, float near, float far, const Input* input_ptr);
        ~Camera(); 

        void setCameraOrientation(const dmath::versor& orientation);
        void setCameraOrientationFromAxisRad(float cam_heading, const dmath::vec3& axis);
        void setSpeed(float speed);
        void setAngularSpeed(float speed);
        void setWindowHandler(const WindowHandler* window_handler);
        void createProjMat(float near, float far, float fovy, float ar);
        void onFramebufferSizeUpdate(int width, int heigth);
        void setForwardVector(const dmath::vec4& vec);
        void setRightVector(const dmath::vec4& vec);
        void setUpVector(const dmath::vec4& vec);
        void setOrbitalCamDistance(double distance);
        void incrementOrbitalCamDistance(double increment);
        void restoreCamOrientation();
        void setOrbitalInclination(double inclination, const dmath::vec3& axis);

        void rotateCameraYaw(double degrees);
        void rotateCameraRoll(double degrees);
        void rotateCameraPitch(double degrees);
        void moveCamera(const dmath::vec3& motion);
        void setCameraPosition(const dmath::vec3& translation);

        bool projChanged() const;
        math::mat4 getViewMatrix() const;
        math::mat4 getProjMatrix() const;
        math::mat4 getCenteredViewMatrix() const;
        dmath::vec3 getCamPosition() const;
        void castRayMousePos(float dist, dmath::vec3& ray_start_world, dmath::vec3& ray_end_world_ext) const;
        int getPrevInputMode() const;
        float getSpeed() const;

        void update();
        void freeCameraUpdate();
        void orbitalCameraUpdate();
};


#endif
