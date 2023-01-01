#ifndef RENDER_CONTEXT_HPP
#define RENDER_CONTEXT_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <thread>
#include <vector>
#include <string>

#include "maths_funcs.hpp"
#include "timing.hpp"


class Camera;
class WindowHandler;
class DebugOverlay;
class Model;
class BaseGUI;
class BasePart;
class Physics;
class DebugDrawer;
class FontAtlas;
class Text2D;
class BaseApp;
class BaseRenderer;

struct object_transform;
struct planet_transform;


/* shader macros */
#define SHADER_PHONG_BLINN 1
#define SHADER_PHONG_BLINN_NO_TEXTURE 2
#define SHADER_TEXT 3
#define SHADER_GUI 4
#define SHADER_PLANET 5
#define SHADER_DEBUG 6
#define SHADER_SPRITE 7
#define SHADER_TEXTURE_NO_LIGHT 8

/* gui modes */
#define GUI_MODE_NONE 0
#define GUI_MODE_EDITOR 1
#define GUI_MODE_PLANETARIUM 2

/* renders states, ie what to render */
#define RENDER_NOTHING 0x0001
#define RENDER_EDITOR 0x0002
#define RENDER_SIMULATION 0x0004
#define RENDER_PLANETARIUM 0x0008


struct notification{
    std::wstring string;
    int ttl;
};


/*
 * This big-ass class deals with the rendering. Basically manages the OpenGL state machine, you
 * probably shouldn't change the state of OpenGL from anywhere outside of this class. State changes
 * should be performed by calling the members of this class. Once the context of the window is 
 * passed to the render thread (in multithreaded applications), any OpenGL calls outside of this
 * thread will probably take no effect. Objects, Planets and other stuff that is drawn on the
 * screen have render methods that can contain OpenGL code (well, more like they must, but the 
 * point is that they will be called from the render thread).
 *
 * All the public methods of this class are thread-safe.
 */

class RenderContext{
    private:
        // shaders //
        GLint m_pb_notex_view_mat, m_pb_notex_proj_mat, m_pb_notex_light_pos;
        GLuint m_pb_notex_shader;

        GLint m_pb_view_mat, m_pb_proj_mat, m_pb_light_pos, m_pb_light_color;
        GLuint m_pb_shader;

        GLint m_text_proj_mat;
        GLuint m_text_shader;

        GLint m_gui_proj_mat;
        GLuint m_gui_shader;

        GLint m_planet_view_mat, m_planet_proj_mat, m_planet_light_pos, m_planet_relative_pos;
        GLuint m_planet_shader;

        GLuint m_debug_shader;
        GLint m_debug_view_mat, m_debug_proj_mat, m_debug_color_location;

        GLint m_sprite_proj_mat;
        GLuint m_sprite_shader;

        GLuint m_tnl_shader;
        GLint m_tnl_view_mat, m_tnl_proj_mat;
        // shaders //

        GLuint m_bound_vao;
        int m_bound_programme;

        std::unique_ptr<DebugOverlay> m_debug_overlay;
        std::unique_ptr<DebugDrawer> m_debug_drawer;

        math::vec4 m_color_clear;
        const Camera* m_camera;
        const WindowHandler* m_window_handler;
        Physics* m_physics;
        const FontAtlas* m_default_atlas;
        BaseApp *m_app;

        math::vec3 m_light_position;

        std::thread m_render_thread;
        bool m_stop;

        int m_fb_width, m_fb_height;
        bool m_update_fb, m_update_projection;

        bool m_debug_draw, m_draw_overlay, m_update_shaders;
        render_timing m_timing;

        // synchronization
        struct render_buffers* m_buffers;

        // gui
        BaseGUI* m_editor_gui;
        BaseGUI* m_planetarium_gui;

        // renderers
        BaseRenderer* m_planetarium_renderer;
        BaseRenderer* m_simulation_renderer;
        BaseRenderer* m_editor_renderer;

        // other ones...
        std::unique_ptr<Text2D> m_notification_text;

        std::vector<struct notification> notifications;

        double m_glfw_time;

        /*
         * Inits OpenGL
         */
        void initGl();

        /*
         * Inits Dear-ImGUI.
         */
        void initImgui();

        /*
         * Called from the "start" method as a separate thread. Contains the main rendering loop.
         */
        void run();

        /*
         * Internal call to render the scene.
         */
        void render();

        /*
         * Renders Dear-ImGUI.
         */
        void renderImGui();

        /*
         * Loads or re-loads the shaders.
         */
        void loadShaders();

        /*
         * Sets the main light position when we are going to render, we have a very simple 
         * rendering pipeline for now.
         */
        void setLightPositionRender();

        void renderBulletDebug(const math::mat4& view_mat);
        void renderNotifications();
    public:
        /*
         * Constructor.
         *
         * @app: raw pointer to the app.
         */
        RenderContext(BaseApp* app);
        ~RenderContext();

        /*
         * Starts and launches the rendering thread. This thread is completely asynchronous from
         * the main thread.
         */
        void start();

        /*
         * Stops the rendering thread
         */
        void stop();

        /*
         * Sets the main light position.
         *
         * @pos: const reference to the light position, relative to the centered camera.
         */
        void setLightPosition(const math::vec3& pos);

        /*
         * Returns a raw pointer to the debug overlay object.
         */
        DebugOverlay* getDebugOverlay();

        /*
         * Binds a program (shader), you should pass one of the shader macros defined at the top of
         * this file (SHADER_PHONG_*).
         *
         * @program: program macro value.
         */
        void useProgram(int program) const;

        /*
         * Binds a buffer array object.
         *
         * @vao: value of the vao.
         */
        void bindVao(GLuint vao) const;

        /*
         * Method called from the WindowManager in case the size of the screen changes.
         *
         * @width: width of the framebuffer.
         * @height: height of the framebuffer.
         */
        void onFramebufferSizeUpdate(int width, int height);

        /*
         * Toggles the rendering of the debug overlay.
         */
        void toggleDebugOverlay();

        /*
         * Toggles the Bullet debug rendering.
         */
        void toggleDebugDraw();

        /*
         * Sets a GUI given a pointer of type BaseGUI and a valid GUI value.
         *
         * @gui_ptr: pointer to the GUI object, should be a derived class from BaseGUI.
         * @gui: GUI that we want to set, has to be one of the valid GUI modes defined above (
         * macros GUI_MODE_*).
         */
        void setGUI(BaseGUI* gui_ptr, short gui);

        /*
         * Sets a renderer given a pointer of type BaseRenderer and a valid render state.
         *
         * @rend_ptr: pointer to the renderer object, should be a derived class from BaseRenderer.
         * @render_state: renderer that we want to set, has to be one of the valid render states
         * defined above (* macros RENDER_*).
         */
        void setRenderer(BaseRenderer* rend_ptr, short render_state);

        /*
         * Reloads the shaders.
         */
        void reloadShaders();

        /*
         * Sets the debug drawer in Physics, the dynamics world needs to have been initialized.
         */
        void setDebugDrawer();

        /*
         * Adds a notification that will be rendered at the center of the screen, in bright green
         * color.
         *
         * @string: constant wchar_t string. Has to be null-terminated.
         * @ttl: time-to-live of the notification.
         */
        void addNotification(const wchar_t* string, int ttl=180);

        /*
         * Sets the default font atlas to be used to render most of the text on the screen.
         *
         * @atlas: pointer to the atlas.
         */
        void setDefaultFontAtlas(const FontAtlas* atlas);

        /*
         * Returns by reference the size of the default framebuffer size (the size of the screen).
         * Used by GUI classes to restore the viewport to the right size.
         *
         * @width: width of the framebuffer.
         * @height: height of the framebuffer.
         */
        void getDefaultFbSize(float& width, float& height) const;

        /*
         * Returns true if Dear-ImGUI wants to process the mouse input.
         */
        bool imGuiWantCaptureMouse() const;

        /*
         * Returns true if Dear-ImGUI wants to process the keyboard input.
         */
        bool imGuiWantCaptureKeyboard() const;

        /*
         * Used to get the location of a uniform variable of a specific shader, since the shader
         * values are internally stored in this class. This method may be thread safe but should
         * ONLY be called from the render methods and the rendering thread, otherwise it may not
         * work.

         * @shader: shader macro.
         * @location: char string with the name of the uniform variable.
         */
        GLuint getUniformLocation(int shader, const char* location) const;

        /*
         * Special context update method for the Planetarium and PlanetRenderer. These applications
         * are single-threaded, so their main thread retains the OpenGL context.
         */
        void contextUpdatePlanetRenderer();
};


#endif
