#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "maths_funcs.hpp"


class WindowHandler;
class Input;


/*
 * This struct holds some camera parameters (such as origin and orientation) to recover the camera
 * position when we change the game state (for example, from planetarium to simulation). This 
 * struct should be used with the getCameraParameters and recoverCameraParameters methods.
 */
struct camera_params{
    float cam_speed;
    dmath::vec3 cam_pos;
    dmath::vec4 fwd, rgt, up;
    dmath::versor cam_orientation;
    double polar_angle, azimuthal_angle, radial_distance, inclination;
    dmath::vec3 inclination_axis;
};


/*
 * Camera class for the main rendering, maybe it's not ideal for other cameras. In the future
 * maybe this class could be a simple class that doesn't deal with input or changes in the 
 * framebuffer size, and then extend in case we need it to deal with input or window size changes.
 */

class Camera{
    private:
        dmath::vec3 m_cam_pos, m_cam_translation;
        dmath::vec4 m_fwd, m_rgt, m_up;
        dmath::versor m_cam_orientation;
        math::mat4 m_proj_mat;
        dmath::mat4 m_view_matrix;
        float m_cam_speed, m_cam_heading_speed, m_near, m_far, m_fovy, m_ar;
        double m_previous_frame_time, m_elapsed_time;
        int m_cam_input_mode, m_prev_cam_input_mode;

        /* orbital camera params */
        double m_polar_angle, m_azimuthal_angle, m_radial_distance, m_inclination;
        dmath::vec3 m_inclination_axis;

        const WindowHandler* m_window_handler;
        const Input* m_input;

        void updateViewMatrix();
        void update();
    public:
        Camera();
        /*
         * Constructor.
         *
         * @pos: starting position of the camera.
         * @fovy: horizontal field of view of the camera, in degrees.
         * @ar: aspect-ratio of the camera.
         * @near: near plane of the camera, in meters.
         * @far: far plane of the camera, in meters.
         * @input_ptr: pointer to the input object of the app. Will change in the future.
         */
        Camera(const dmath::vec3& pos, float fovy, float ar, float near, float far, const Input* input_ptr);
        ~Camera(); 

        /*
         * Sets the orientation of the camera given a versor.
         *
         * @orientation: versor with the new orientation.
         */
        void setCameraOrientation(const dmath::versor& orientation);

        /*
         * Sets the orientation of the camera given the heading and the camera axis. I suppose the
         * camera heading is the roll?
         *
         * @cam_heading: camera heading, most likely the roll.
         * @axis: camera orientation.
         */
        void setCameraOrientationFromAxisRad(float cam_heading, const dmath::vec3& axis);

        /*
         * Sets the free camera speed, in m/s.
         *
         * @speed: camera speed in m/s.
         */
        void setSpeed(float speed);

        /*
         * Angular speed of the free camera (yaw and pitch) when the keyboard is used, in rad/s.
         * 
         * @speed: angular speed, in rad/s.
         */
        void setAngularSpeed(float speed);

        /*
         * This will be gone in the future, better ignore it :).
         */
        void setWindowHandler(const WindowHandler* window_handler);

        /*
         * Creates a new projection matrix.
         * 
         * @near: near plane, in meters.
         * @far: far plane, in meters.
         * @fovy: horizontal field of view, in degrees.
         */
        void createProjMat(float near, float far, float fovy);

        /*
         * Method called by the window handler in case the framebuffer size changes. Could be
         * used to change the fb size in case this class is used for other cameras. The new size
         * is used to update the aspect-ratio of the projection matrix.
         *
         * @width: new width of the framebuffer.
         * @heigth: new heigth of the framebuffer.
         */
        void onFramebufferSizeUpdate(int width, int heigth);

        /*
         * These functions reset the forward/right/up vectors, should only be used in the free
         * camera, the orbital camera computes its own vectors.
         */
        void setForwardVector(const dmath::vec4& vec);
        void setRightVector(const dmath::vec4& vec);
        void setUpVector(const dmath::vec4& vec);

        /*
         * Sets the distance to the target in the orbital camera.
         *
         * @distance: distance from the camera to the target, in meters.
         */
        void setOrbitalCamDistance(double distance);

        /*
         * Increments the distance between the camera and the target in the orbital camera.
         *
         * @incremet: increment, positive or negative, in meters.
         */
        void incrementOrbitalCamDistance(double increment);

        /*
         * Returns the distance between the orbital camera and the target.
         */
        double getOrbitalCamDistance() const;

        /*
         * Test method, should be used to recover the orientation of the camera when we change
         * from the orbital camera to the free camera. Doesn't work.
         */
        void restoreCamOrientation();

        /*
         * Sets how much the orbital camera should be inclined. For example, when the orbital 
         * camera mode is set to "surface" in the class Player ("ORBITAL_CAM_MODE_SURFACE"),
         * the right vector of the camera should always be perpendicular to the vector that crosses
         * the center of the planet and the vessel. When the camera mode is set to
         * ORBITAL_CAM_MODE_ORBITAL, we apply no rotation so the right vector should be parallel
         * to the earth's orbital plane around the sun. In any case, this doesn't seem to be
         * working right, so it'll change in the future.
         *
         * @inclination: inclination angle, in radians.
         * @axis: axis of the rotation.
         */
        void setOrbitalInclination(double inclination, const dmath::vec3& axis);

        /*
         * Methods used to rotate the free camera along one of its axis.
         *
         * @rads: amount of rotation, in radians.
         */
        void rotateCameraYaw(double rads);
        void rotateCameraRoll(double rads);
        void rotateCameraPitch(double rads);

        /*
         * Moves the camera given a vector (m_cam_pos += motion).
         *
         * @motion: vector to be added to the camera translation.
         */
        void moveCamera(const dmath::vec3& motion);

        /*
         * Resets the camera position.
         *
         * @translation: new position of the camera.
         */
        void setCameraPosition(const dmath::vec3& translation);

        /*
         * Getters for the view and projection matrices. Note that getViewMatrix returns a single
         * precision view matrix, which might result in loss of precision. getCenteredViewMatrix
         * returns the view matrix center at the origin, which is used for rendering.
         */
        math::mat4 getViewMatrix() const;
        const math::mat4& getProjMatrix() const;
        math::mat4 getCenteredViewMatrix() const;

        /*
         * Returns the position of the camera, in double precision.
         */
        const dmath::vec3& getCamPosition() const;

        /*
         * Returns the start and end of a ray casted from the current mouse position with 
         * length "dist".
         *
         * @dist: length of the ray to be casted
         * @ray_start_world: where the start of the ray will be saved.
         * @ray_end_world_ext: where the end of the ray will be saved.
         */
        void castRayMousePos(float dist, dmath::vec3& ray_start_world, dmath::vec3& ray_end_world_ext) const;

        /* 
         * Returns the last glfw input mode of the cursor, used by some functions to avoid, for
         * example, casting rays when the rmb is released after the camera is moved with the mouse.
         */
        int getPrevInputMode() const;

        /*
         * Returns the speed of the orbital camera, in m/s.
         */
        float getSpeed() const;

        /*
         * Updates the free camera, including the input.
         */
        void freeCameraUpdate();

        /*
         * Updates the orbital camera, including the input.
         */
        void orbitalCameraUpdate();

        /*
         * Fills the params struct with the current parameters of the camera.
         *
         * @params: reference to a camera parameter struct where the parameters of the camera will 
         * be stored.
         */
        void getCameraParams(struct camera_params& params) const;

        /*
         * Recovers the camera parameters from the given params canera_params struct.
         *
         * @params: constant reference to a camera parameters struct.
         */
        void recoverCameraParams(const struct camera_params& params);
};


#endif
