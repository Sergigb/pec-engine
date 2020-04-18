#ifndef DEBUGOVERLAY_HPP
#define DEBUGOVERLAY_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <iomanip>

#include "Text2D.hpp"
#include "Camera.hpp"
#include "WindowHandler.hpp"
#include "gl_utils.hpp"

class DebugOverlay{
    private:
        std::unique_ptr<Text2D> m_text_debug;
        std::unique_ptr<Text2D> m_text_dynamic_text;
        int m_rendered_obj;        
        double m_physics_load_time, m_physics_sleep_time;
    public:
        DebugOverlay(int fb_width, int fb_height, GLuint shader);
        ~DebugOverlay();

        void setRenderedObjects(int n);
        void setPhysicsTimes(double physics_load_time, double physics_sleep_time);
        void onFramebufferSizeUpdate(int fb_width, int fb_height);

        void render();
};

#endif