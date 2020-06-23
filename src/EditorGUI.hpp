#ifndef EDITORGUI_HPP
#define EDITORGUI_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "WindowHandler.hpp"
#include "BaseGUI.hpp"
#include "common.hpp"
#include "FontAtlas.hpp"
#include "RenderContext.hpp"

// not sure if it's a good idea to hardcode this
#define EDITOR_GUI_VERTEX_NUM 7
#define EDITOR_GUI_INDEX_NUM 12
#define EDITOR_GUI_LP_W 150.0
#define EDITOR_GUI_TP_H 30.0


class EditorGUI : public BaseGUI{
    private:
        bool m_fb_update, m_init;

        GLuint m_vao, m_vbo_vert, m_vbo_tex, m_vbo_ind;
        GLuint m_shader_programme;

        math::mat4 m_projection;

        FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
        const WindowHandler* m_window_handler;

        void updateBuffers();
    public:
        EditorGUI();
        EditorGUI(const WindowHandler* window_handler, FontAtlas* atlas, GLuint shader, const RenderContext* render_context);
        ~EditorGUI();

        void onFramebufferSizeUpdate();
        void render();
        void update();
};


#endif
