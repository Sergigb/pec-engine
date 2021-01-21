#ifndef EDITORGUI_HPP
#define EDITORGUI_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <map>
#include <memory>

#include "BaseGUI.hpp"
#include "maths_funcs.hpp"


class BasePart;
class FontAtlas;
class RenderContext;
class Input;
class PartsPanelGUI;
class Text2D;
class StagingPanelGUI;


// not sure if it's a good idea to hardcode this
#define EDITOR_GUI_VERTEX_NUM 31
#define EDITOR_GUI_INDEX_NUM 48

#define EDITOR_GUI_LP_W 300.0f
#define EDITOR_GUI_TP_H 40.0f
#define EDITOR_GUI_N_TOP_BUTTONS 3 // changing this won't add buttons cuz the array sizes are fixed
#define EDITOR_GUI_PP_MARGIN 15.0f
#define EDITOR_GUI_PP_LOW_MARGIN 100.0f

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

#define DELETE_AREA_ORIGIN 15.0f
#define DELETE_AREA_MARGIN 15.0f
#define DELETE_AREA_COLOR 0.0f, 0.0f, 0.0f, 0.0f
#define DELETE_AREA_MOUSEOVER -0.2f, -0.2f, -0.2f, 0.0f

#define TAB_OPTION_NONE 0
#define TAB_OPTION_PARTS 1
#define TAB_OPTION_STAGING 2
#define TAB_NUMBER 2
#define TAB_WIDTH 75.0f
#define TAB_HEIGTH 25.0f
#define TAB_COLOR_SELECTED 0.25f, 0.25f, 0.25f, 1.0
#define TAB_COLOR_UNSELECTED  0.2, 0.2, 0.2, 1.0
#define TAB_COLOR_MOUSEOVER 0.225f, 0.225f, 0.225f, 1.0

// actions related to the gui
#define EDITOR_ACTION_NONE 0
#define EDITOR_ACTION_FOCUS 1
#define EDITOR_ACTION_OBJECT_PICK 2
#define EDITOR_ACTION_BUTTON 3 // well see about this
#define EDITOR_ACTION_DELETE 4


class EditorGUI : public BaseGUI{
    private:
        bool m_fb_update, m_init;
        float m_fb_height, m_fb_width;
        double m_mouse_x, m_mouse_y;
        int m_button_select;
        bool m_button_status[EDITOR_GUI_N_TOP_BUTTONS];
        char m_tab_option;

        GLuint m_vao, m_vbo_vert, m_vbo_tex, m_vbo_ind, m_vbo_clr;
        GLuint m_texture_atlas;
        GLuint m_disp_location;

        // parts panel, this is related to the drawing on the panel, not the contents
        GLuint m_left_panel_vao, m_left_panel_vbo_vert, m_left_panel_vbo_tex, m_left_panel_vbo_clr;
        std::unique_ptr<PartsPanelGUI> m_parts_panel;
        std::unique_ptr<StagingPanelGUI> m_staging_panel;

        math::mat4 m_projection;

        std::unique_ptr<Text2D> m_text_debug;

        const FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
        const Input* m_input;

        void updateBuffers();
        void updateButtons();
        void updateTabsColor();
        void updateDeleteArea();
        void updateTopButtons();
        void setButtonColor(float r ,float g, float b, float a, GLintptr offset);
    public:
        EditorGUI();
        EditorGUI(const FontAtlas* atlas, const RenderContext* render_context, const Input* input);
        ~EditorGUI();

        void setMasterPartList(const std::map<std::uint32_t, std::unique_ptr<BasePart>>* master_parts_list);

        void onFramebufferSizeUpdate();
        void render();
        int update();

        const std::unique_ptr<BasePart>* getPickedObject() const;
};


#endif
