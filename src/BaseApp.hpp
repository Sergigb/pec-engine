#ifndef BASEAPP_HPP
#define BASEAPP_HPP

#include <memory>
#include <vector>
#include <mutex>

#include "Camera.hpp"
#include "Input.hpp"
#include "WindowHandler.hpp"
#include "Frustum.hpp"
#include "BtWrapper.hpp"
#include "Object.hpp"
#include "RenderContext.hpp"
#include "buffers.hpp"
#include "BasePart.hpp"
#include "Model.hpp"
#include "maths_funcs.hpp"

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

// base app for future shenanigans

class BaseApp{
    private:
        void init(int gl_width, int gl_height);
    protected:
        std::unique_ptr<Input> m_input;
        std::unique_ptr<Camera> m_camera;
        std::unique_ptr<WindowHandler> m_window_handler;
        std::unique_ptr<Frustum> m_frustum;
        std::unique_ptr<RenderContext> m_render_context;
        std::unique_ptr<BtWrapper> m_bt_wrapper;

        std::vector<std::unique_ptr<btCollisionShape>> m_collision_shapes;
        std::vector<std::unique_ptr<Object>> m_objects;

        /* Note: this is a tricky situation. If BasePart inherits from Object, why not
        include the parts in m_objects (and use virtual functions if needed)? As now,
        I want to have parts in a different vector for when we are in the editor and we
        need to draw the attachment points. I dont want to include opengl code in BasePart
        or Object, so the render context should be the one calling the opengl functions
        and draw the attachment points (and we need to know that the Object is actually 
        a BasePart to call the corresponding method). Also, I want to avoid using dynamic
        casts on m_objects (ew).

        This will change when we add the object trees, kinematic objects, and other more 
        complex crap.*/
        std::vector<std::unique_ptr<BasePart>> m_parts;

        // buffers used to synchronize the physics and rendering
        struct render_buffers m_buffers;
    public:
        BaseApp();
        BaseApp(int gl_width, int gl_height);
        virtual ~BaseApp();

        virtual void run();
};


#endif
