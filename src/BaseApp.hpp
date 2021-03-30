#ifndef BASEAPP_HPP
#define BASEAPP_HPP

#include <memory>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "multithreading.hpp"
#include "buffers.hpp"

// base app for future shenanigans

class Camera;
class Input;
class WindowHandler;
class Frustum;
class Physics;
class RenderContext;
class AssetManager;
class Player;
class GameEditor;


class BaseApp{
    private:
        void init(int gl_width, int gl_height);
    protected:
        // in the future there should be a base class that we make a friend of this one, and derive the others from that one
        friend class GameEditor;
        friend class AssetManager;
        friend class RenderContext;

        std::unique_ptr<Input> m_input;
        std::unique_ptr<Camera> m_camera;
        std::unique_ptr<WindowHandler> m_window_handler;
        std::unique_ptr<Frustum> m_frustum;
        std::unique_ptr<RenderContext> m_render_context;
        std::unique_ptr<Physics> m_physics;
        std::unique_ptr<AssetManager> m_asset_manager;
        std::unique_ptr<Player> m_player;

        // buffers used to synchronize the physics and rendering
        struct render_buffers m_buffers;
        struct thread_monitor m_thread_monitor;
    public:
        BaseApp();
        BaseApp(int gl_width, int gl_height);
        virtual ~BaseApp();

        virtual void run();
};


#endif
