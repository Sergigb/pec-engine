#ifndef BASEAPP_HPP
#define BASEAPP_HPP

#include <memory>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "multithreading.hpp"
#include "buffers.hpp"


class Camera;
class Input;
class WindowHandler;
class Frustum;
class Physics;
class RenderContext;
class AssetManager;
class Player;
class GameEditor;


/*
 * Base application class, should be inherited by any application class. Has most of the essential
 * stuff to display, render, and manage whatever we need.
 */

class BaseApp{
    private:
        void displayLoadingScreen();

        void init(int gl_width, int gl_height);
    protected:
        friend class GameEditor;
        friend class GamePlanetarium;
        friend class AssetManager;
        friend class RenderContext;
        friend class Physics;

        std::unique_ptr<Input> m_input;
        std::unique_ptr<Camera> m_camera;
        std::unique_ptr<WindowHandler> m_window_handler;
        std::unique_ptr<Frustum> m_frustum;
        std::unique_ptr<RenderContext> m_render_context;
        std::unique_ptr<Physics> m_physics;
        std::unique_ptr<AssetManager> m_asset_manager;
        std::unique_ptr<Player> m_player;

        short m_gui_mode, m_render_state;

        /* buffers used to synchronize the physics and rendering */
        struct render_buffers m_buffers;
        struct thread_monitor m_thread_monitor;
    public:
        BaseApp();
        BaseApp(int gl_width, int gl_height);
        virtual ~BaseApp();

        virtual void run();

        /*
         * Returns the GUI mode of the application, used by RenderContext to know which GUI it
         * should be rendering. The values are defined in that class (GUI_MODE_*).
         */
        short getGUIMode() const;

        /*
         * Returns the render state of the application (editor, simulation, etc), the values are
         * defined in RenderContext (RENDER_*).
         */
        short getRenderState() const;
};


#endif
