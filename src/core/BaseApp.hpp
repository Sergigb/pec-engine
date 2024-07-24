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
class Predictor;


/*
 * Base application class, should be inherited by any application class. Has most of the essential
 * stuff to display, render, and manage whatever we need.
 */

class BaseApp{
    private:
        void displayLoadingScreen();

        void init(int gl_width, int gl_height);
    protected:
        std::unique_ptr<Input> m_input;
        std::unique_ptr<Camera> m_camera;
        std::unique_ptr<WindowHandler> m_window_handler;
        std::unique_ptr<Frustum> m_frustum;
        std::unique_ptr<RenderContext> m_render_context;
        std::unique_ptr<Physics> m_physics;
        std::unique_ptr<AssetManager> m_asset_manager;
        std::unique_ptr<Player> m_player;
        std::unique_ptr<Predictor> m_predictor;

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

        /*
         * Sets the render state of the application (editor, simulation, etc), the values are 
         * defined in the RenderContext (RENDER_*).
         */
        void setRenderState(short state);

        /*
         * Sets the GUI mode of the application, used by RenderContext to know which GUI it
         * should be rendering. The values are defined in that class (GUI_MODE_*).
         */
        void setGUIMode(short mode);

        /*
         * Returns a pointer to the render buffers of the app.
         */
        struct render_buffers* getRenderBuffers();

        /*
         * Returns a pointer to the thread monitor of the app.
         */
        struct thread_monitor* getThreadMonitor();

        /*
         * Returns a constant pointer to the Input object of the app.
         */
        const Input* getInput() const;

        /*
         * Returns a pointer to the app Input object.
         */
        Input* getInput();

        /*
         * Returns a constant pointer to the Camera object of the app.
         */
        const Camera* getCamera() const;

        /*
         * Returns a pointer to the Camera object of the app.
         */
        Camera* getCamera();

        /*
         * Returns a constant pointer to the WindowHandler object of the app.
         */
        const WindowHandler* getWindowHandler() const;

        /*
         * Returns a pointer to the WindowHandler object of the app.
         */
        WindowHandler* getWindowHandler();

        /*
         * Returns a constant pointer to the Frustum object of the app.
         */
        const Frustum* getFrustum() const;

        /*
         * Returns a pointer to the Frustum object of the app.
         */
        Frustum* getFrustum();

        /*
         * Returns a constant pointer to the RenderContext object of the app.
         */
        const RenderContext* getRenderContext() const;

        /*
         * Returns a pointer to the RenderContext object of the app.
         */
        RenderContext* getRenderContext();

        /*
         * Returns a constant pointer to the Physics object of the app.
         */
        const Physics* getPhysics() const;

        /*
         * Returns a pointer to the Physics object of the app.
         */
        Physics* getPhysics();

        /*
         * Returns a constant pointer to the AssetManager object of the app.
         */
        const AssetManager* getAssetManager() const;

        /*
         * Returns a pointer to the AssetManager object of the app.
         */
        AssetManager* getAssetManager();

        /*
         * Returns a constant pointer to the Player object of the app.
         */
        const Player* getPlayer() const;

        /*
         * Returns a pointer to the Player object of the app.
         */
        Player* getPlayer();

        /*
         * Returns a constant pointer to the Predictor object of the app.
         */
        const Predictor* getPredictor() const;

        /*
         * Returns a pointer to the Predictor object of the app.
         */
        Predictor* getPredictor();
};


#endif
