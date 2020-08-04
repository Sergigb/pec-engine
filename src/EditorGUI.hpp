#ifndef EDITORGUI_HPP
#define EDITORGUI_HPP

#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb/stb_image.h>

#include "WindowHandler.hpp"
#include "BaseGUI.hpp"
#include "common.hpp"
#include "FontAtlas.hpp"
#include "RenderContext.hpp"
#include "Input.hpp"
#include "BasePart.hpp"
#include "log.hpp"
#include "PartsPanelGUI.hpp"
#include "Text2D.hpp"

// not sure if it's a good idea to hardcode this
#define EDITOR_GUI_VERTEX_NUM 19
#define EDITOR_GUI_INDEX_NUM 30
#define EDITOR_GUI_LP_W 300.0f
#define EDITOR_GUI_TP_H 40.0f
#define EDITOR_GUI_N_BUTTONS 3 // changing this won't add buttons cuz the array sizes are fixed
#define EDITOR_GUI_PP_MARGIN 30.0f

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

// actions related to the gui
#define EDITOR_ACTION_NONE 0
#define EDITOR_ACTION_OBJECT_PICK 1
#define EDITOR_ACTION_BUTTON 2 // well see about this



class EditorGUI : public BaseGUI{
    private:
        bool m_fb_update, m_init;
        float m_fb_height, m_fb_width;
        int m_button_mouseover, m_button_select;
        bool m_button_status[EDITOR_GUI_N_BUTTONS];

        // used by the render thread
        int m_last_button_color; // last button that changed color
        bool m_button_color_status[EDITOR_GUI_N_BUTTONS];

        GLuint m_vao, m_vbo_vert, m_vbo_tex, m_vbo_ind, m_vbo_clr;
        GLuint m_texture_atlas;
        GLuint m_gui_shader;
        GLuint m_disp_location;

        // parts panel, this is related to the drawing on the panel, not the contents
        GLuint m_parts_panel_vao, m_parts_panel_vbo_vert, m_parts_panel_vbo_tex, m_parts_panel_vbo_clr;
        std::unique_ptr<PartsPanelGUI> m_parts_panel;

        math::mat4 m_projection;

        std::unique_ptr<Text2D> m_text_debug;

        const FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
        const Input* m_input;
        const std::map<int, std::unique_ptr<BasePart>>* m_master_parts_list;

        void updateBuffers();
        void updateButtons();
        void colorButton(const GLfloat* color_array, int button);
    public:
        EditorGUI();
        EditorGUI(const FontAtlas* atlas, const RenderContext* render_context, const Input* input);
        ~EditorGUI();

        void setMasterPartList(const std::map<int, std::unique_ptr<BasePart>>* master_parts_list);

        void onFramebufferSizeUpdate();
        void render();
        void update();
};


#endif
