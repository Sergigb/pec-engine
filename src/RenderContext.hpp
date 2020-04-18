#ifndef RENDER_CONTEXT_HPP
#define RENDER_CONTEXT_HPP
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include <mutex>

#include "gl_utils.hpp"
#include "Camera.hpp"
#include "WindowHandler.hpp"
#include "DebugOverlay.hpp"
#include "Object.hpp"
#include "common.hpp"


#define SHADER_PHONG_BLINN 1
#define SHADER_PHONG_BLINN_NO_TEXTURE 2
#define SHADER_TEXT 3


class RenderContext{
    private:
        // shaders //
        GLint m_pb_notex_view_mat, m_pb_notex_proj_mat, m_pb_notex_light_pos;
        GLuint m_pb_notex_shader;

        GLint m_pb_view_mat, m_pb_proj_mat, m_pb_light_pos, m_pb_light_color;
        GLuint m_pb_shader;

        GLint m_text_proj_mat;
        GLuint m_text_shader;

        // shaders //

        std::unique_ptr<DebugOverlay> m_debug_overlay;

        float m_bg_r, m_bg_g, m_bg_b, m_bg_a;
        const Camera* m_camera;
        const WindowHandler* m_window_handler;

        std::vector<std::unique_ptr<Object>>* m_objects;

        // synchronization
        using buffer = std::vector<object_transform>;
        const buffer* m_buffer1;
        const buffer* m_buffer2;
        std::mutex* m_buffer1_lock;
        std::mutex* m_buffer2_lock;
        const buffer_manager* m_last_updated;

        void initGl();
    public:
        RenderContext(const Camera* camera, const WindowHandler* window_handler, const buffer* buffer1, const buffer* buffer2,
                      std::mutex* buff1_lock, std::mutex* buff2_lock, const buffer_manager* manager);
        ~RenderContext();

        void render(bool render_asynch);

        void setLightPosition(const math::vec3& pos) const;
        void setObjectVector(std::vector<std::unique_ptr<Object>>* objects);
        void setDebugOverlayPhysicsTimes(double physics_load_time, double physics_sleep_time);

        GLuint getShader(int shader) const;
};


#endif
