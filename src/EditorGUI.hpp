#ifndef EDITORGUI_HPP
#define EDITORGUI_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "WindowHandler.hpp"
#include "BaseGUI.hpp"
#include "common.hpp"
#include "FontAtlas.hpp"
#include "RenderContext.hpp"
#include "Input.hpp"

// not sure if it's a good idea to hardcode this
#define EDITOR_GUI_VERTEX_NUM 19
#define EDITOR_GUI_INDEX_NUM 30
#define EDITOR_GUI_LP_W 300.0
#define EDITOR_GUI_TP_H 40.0
#define EDITOR_GUI_N_BUTTONS 3 // changing this won't add buttons cuz the array sizes are fixed

// colors are defined per vertex
#define EDITOR_GUI_PANEL_COLOR 0.15, 0.15, 0.15, 1.0

#define BUTTON_PAD_X 20
#define BUTTON_PAD_Y 5
#define BUTTON_SIZE_X 100
#define BUTTON_SIZE_Y 30

#define BUTTON_COLOR_DEFAULT 0.25, 0.25, 0.25, 1.0
#define BUTTON_COLOR_MOUSEOVER 0.35, 0.35, 0.35, 1.0
#define BUTTON_COLOR_SELECTED 0.0, 0.0, 0.9, 1.0
#define BUTTON_COLOR_SELECTED_MOUSEOVER 0.0, 0.0, 0.7, 1.0



class EditorGUI : public BaseGUI{
    private:
        bool m_fb_update, m_init;
        float m_fb_height_f, m_fb_width_f;
        int m_button_mouseover, m_button_select;
        bool m_button_status[EDITOR_GUI_N_BUTTONS];

        // used by the render thread
        int m_last_button_color; // last button that changed color
        bool m_button_color_status[EDITOR_GUI_N_BUTTONS];

        GLuint m_vao, m_vbo_vert, m_vbo_tex, m_vbo_ind, m_vbo_clr;
        GLuint m_shader_programme;

        math::mat4 m_projection;

        FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
        const Input* m_input;

        void updateBuffers();
        void updateButtons();
        void colorButton(const GLfloat* color_array, int button);
    public:
        EditorGUI();
        EditorGUI(const WindowHandler* window_handler, FontAtlas* atlas, GLuint shader, const RenderContext* render_context, const Input* input);
        ~EditorGUI();

        void onFramebufferSizeUpdate();
        void render();
        void update();
};


#endif
