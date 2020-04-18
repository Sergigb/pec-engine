#ifndef APP_HPP
#define APP_HPP

#include <memory>
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
        std::unique_ptr<Input> m_input;
        std::unique_ptr<Camera> m_camera;
        std::unique_ptr<WindowHandler> m_window_handler;
        std::unique_ptr<Frustum> m_frustum;
        std::unique_ptr<RenderContext> m_render_context;
        std::unique_ptr<BtWrapper> m_bt_wrapper;

        // some models for testing
        std::unique_ptr<Model> m_cube_model;
        std::unique_ptr<Model> m_terrain_model;
        std::unique_ptr<Model> m_sphere_model;

        std::vector<std::unique_ptr<btCollisionShape>> m_collision_shapes;
        std::vector<std::unique_ptr<Object>> m_objects;

        // buffers used to synchronize the physics and rendering
        std::vector<object_transform> m_buffer1;
        std::vector<object_transform> m_buffer2;
        std::mutex m_buffer1_lock;
        std::mutex m_buffer2_lock;
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
