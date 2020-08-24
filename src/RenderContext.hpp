#ifndef RENDER_CONTEXT_HPP
#define RENDER_CONTEXT_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>
#include <chrono>

#include "gl_utils.hpp"
#include "Camera.hpp"
#include "WindowHandler.hpp"
#include "DebugOverlay.hpp"
#include "Object.hpp"
#include "common.hpp"
#include "BasePart.hpp"
#include "Model.hpp"
#include "BaseGUI.hpp"

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>


#define SHADER_PHONG_BLINN 1
#define SHADER_PHONG_BLINN_NO_TEXTURE 2
#define SHADER_TEXT 3
#define SHADER_GUI 4

// gui modes, only editor for now
#define GUI_MODE_NONE 0
#define GUI_MODE_EDITOR 1


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
        // shaders //

        GLuint m_bound_vao;
        GLuint m_bound_programme;

        std::unique_ptr<DebugOverlay> m_debug_overlay;

        float m_bg_r, m_bg_g, m_bg_b, m_bg_a;
        const Camera* m_camera;
        const WindowHandler* m_window_handler;
        
        std::unique_ptr<Model> m_att_point_model;
        math::mat4 m_att_point_scale;

        std::thread m_render_thread;
        bool m_pause, m_stop;

        int m_fb_width, m_fb_height;
        bool m_update_fb, m_update_projection;

        bool m_draw_overlay;
        double m_rscene_acc_load_time, m_rgui_acc_load_time;

        // synchronization
        struct render_buffers* m_buffers;

        // gui
        BaseGUI* m_editor_gui;
        // other ones...
        short m_gui_mode;

        double m_glfw_time;

        void initGl();
        void initImgui();
        void run();
        void render();
        void renderAttPoints(const BasePart* part, int& num_rendered, const math::mat4& body_transform);
        void testImgui();
    public:
        RenderContext(const Camera* camera, const WindowHandler* window_handler, render_buffers* buff_manager);
        ~RenderContext();
        
        void start();
        void stop();
        void pause(bool pause);

        void setLightPosition(const math::vec3& pos) const;
        void setDebugOverlayTimes(double physics_load_time, double logic_load_time, double logic_sleep_time);
        void setAttPointModel(std::unique_ptr<Model>* att_point_model);
        void useProgram(GLuint program) const;
        void bindVao(GLuint vao) const;
        void onFramebufferSizeUpdate(int width, int height);
        void toggleDebugOverlay();
        void setEditorGUI(BaseGUI* editor_ptr);
        void setEditorMode(short mode);

        GLuint getShader(int shader) const;
        GLuint getBoundShader() const;
        GLuint getBoundVao() const;
        void getDefaultFbSize(float& width, float& height) const;

};


#endif
