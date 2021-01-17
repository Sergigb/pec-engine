#ifndef RENDER_CONTEXT_HPP
#define RENDER_CONTEXT_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <thread>
#include <vector>

#include "maths_funcs.hpp"


class Camera;
class WindowHandler;
class DebugOverlay;
class Model;
class BaseGUI;
class BasePart;
class BtWrapper;
class DebugDrawer;

struct object_transform;


#define SHADER_PHONG_BLINN 1
#define SHADER_PHONG_BLINN_NO_TEXTURE 2
#define SHADER_TEXT 3
#define SHADER_GUI 4
#define SHADER_PLANET 5
#define SHADER_DEBUG 6

// gui modes, only editor for now
#define GUI_MODE_NONE 0
#define GUI_MODE_EDITOR 1

// renders states, ie what to render
#define RENDER_NOTHING 0
#define RENDER_EDITOR 1
#define RENDER_UNIVERSE 2


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
        GLint m_debug_view_mat, m_debug_proj_mat;
        // shaders //

        GLuint m_bound_vao;
        int m_bound_programme;

        std::unique_ptr<DebugOverlay> m_debug_overlay;
        std::unique_ptr<DebugDrawer> m_debug_drawer;

        math::vec4 m_color_clear;
        const Camera* m_camera;
        const WindowHandler* m_window_handler;
        BtWrapper* m_bt_wrapper;
        
        std::unique_ptr<Model> m_att_point_model;
        math::mat4 m_att_point_scale;
        math::vec3 m_light_position;

        std::thread m_render_thread;
        bool m_pause, m_stop;

        int m_fb_width, m_fb_height;
        bool m_update_fb, m_update_projection;

        bool m_debug_draw, m_draw_overlay, m_update_shaders;
        double m_rscene_acc_load_time, m_rgui_acc_load_time, m_rimgui_acc_load_time;

        // synchronization
        struct render_buffers* m_buffers;

        // gui
        BaseGUI* m_editor_gui;
        // other ones...
        short m_gui_mode;

        double m_glfw_time;

        short m_render_state;

        void initGl();
        void initImgui();
        void run();
        void render();
        void renderAttPoints(const BasePart* part, const math::mat4& body_transform);
        void renderImGui();
        void loadShaders();
        void setLightPositionRender();

        int renderSceneEditor();
        int renderSceneUniverse();
        int renderObjects(bool render_att_points, const std::vector<object_transform>* buff, const math::mat4* view_mat);
        void renderBulletDebug(const math::mat4* view_mat);
    public:
        RenderContext(const Camera* camera, const WindowHandler* window_handler, render_buffers* buff_manager);
        ~RenderContext();
        
        void start();
        void stop();
        void pause(bool pause);

        void setLightPosition(const math::vec3& pos);
        void setDebugOverlayTimes(double physics_load_time, double logic_load_time, double logic_sleep_time);
        void setAttPointModel(std::unique_ptr<Model>* att_point_model);
        void useProgram(int program) const;
        void bindVao(GLuint vao) const;
        void onFramebufferSizeUpdate(int width, int height);
        void toggleDebugOverlay();
        void toggleDebugDraw();
        void setEditorGUI(BaseGUI* editor_ptr);
        void setGUIMode(short mode);
        void reloadShaders();
        void setDebugDrawer(BtWrapper* bt_wrapper);
        void setRenderState(char state);

        int getBoundShader() const;
        GLuint getBoundVao() const;
        void getDefaultFbSize(float& width, float& height) const;
        bool imGuiWantCaptureMouse() const;
        bool imGuiWantCaptureKeyboard() const;
        GLuint getUniformLocation(int shader, const char* location) const;

        void contextUpdatePlanetarium();
};


#endif
