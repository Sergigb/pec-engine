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
#include "multithreading.hpp"
#include "AssetManager.hpp"

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
        std::unique_ptr<AssetManager> m_asset_manager;

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
