#ifndef APP_HPP
#define APP_HPP

#include <vector>
#include <mutex>

#include "maths_funcs.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "WindowHandler.hpp"
#include "Text2D.hpp"
#include "Frustum.hpp"
#include "Model.hpp"
#include "BtWrapper.hpp"
#include "Object.hpp"
#include "RenderContext.hpp"
#include "common.hpp"

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


class App{
    private:
        Input* m_input;
        Camera* m_camera;
        WindowHandler* m_window_handler;
        Frustum* m_frustum;
        RenderContext* m_render_context;
        BtWrapper* m_bt_wrapper;

        // some models for testing
        Model* m_cube_model;
        Model* m_terrain_model;
        Model* m_sphere_model;

        btAlignedObjectArray<btCollisionShape*> m_collision_shapes;
        std::vector<Object*> m_objects;

        // buffers used to synchronize the physics and rendering
        std::vector<object_transform> m_buffer1;
        std::vector<object_transform> m_buffer2;
        std::mutex m_buffer1_lock;
        std::mutex m_buffer2_lock;
        std::mutex m_manager_lock;
        buffer_manager m_last_updated;

        // game state
        bool m_physics_pause;
        Object* m_picked_obj;

        void init(int gl_width, int gl_height);
        void modelsInit();
        void objectsInit();
    public:
        App();
        App(int gl_width, int gl_height);
        ~App();

        void run();
};


#endif
