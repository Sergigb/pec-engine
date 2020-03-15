#ifndef DEBUGOVERLAY_HPP
#define DEBUGOVERLAY_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Text2D.hpp"
#include "Camera.hpp"
#include "WindowHandler.hpp"
#include "gl_utils.hpp"

class DebugOverlay{
    private:
        Text2D* m_text_debug;
        Text2D* m_text_fps;
        Text2D* m_text_rendered_objects;
        const Camera* m_camera;
        const WindowHandler* m_window_handler;
        int m_rendered_obj;        
    public:
        DebugOverlay(const Camera* camera, const WindowHandler* window_handler);
        ~DebugOverlay();

        void setRenderedObjects(int n);
        void render();
};

#endif