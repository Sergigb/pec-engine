#ifndef EDITORGUI_HPP
#define EDITORGUI_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <memory>
#include <tuple>

#include "../BaseGUI.hpp"
#include "../../core/maths_funcs.hpp"


class BasePart;
class FontAtlas;
class RenderContext;
class Input;
class PartsPanelGUI;
class Text2D;
class StagingPanelGUI;
class Sprite;
class BaseApp;
class ImFont;


// actions related to the gui
#define EDITOR_ACTION_NONE 0
#define EDITOR_ACTION_FOCUS 1
#define EDITOR_ACTION_OBJECT_PICK 2
#define EDITOR_ACTION_DELETE 3
#define EDITOR_ACTION_SYMMETRY_SIDES 4
#define EDITOR_ACTION_TOGGLE_ALIGN 5
#define EDITOR_ACTION_CLEAR_SCENE 6
#define EDITOR_ACTION_CHANGE_NAME 7

#define MAX_LEN_VESSEL_NAME 32


class EditorGUI : public BaseGUI{
    private:
        short m_symmetric_sides, m_max_symmetric_sides;
        bool m_radial_align;
        char m_vessel_name[MAX_LEN_VESSEL_NAME];
        int m_action;
        bool m_init;
        std::uint32_t m_picked_object;
        std::tuple<int, int> m_action_swap;
        BasePart* m_highlight_part;

        const std::unordered_map<std::uint32_t, std::unique_ptr<BasePart>>* m_master_parts_list;

        //GLuint m_vao, m_vbo_vert, m_vbo_tex, m_vbo_ind, m_vbo_clr;
        GLuint m_texture_atlas;
        int m_tex_size_x, m_tex_size_y;

        std::unique_ptr<Text2D> m_main_text;

        const FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
        const Input* m_input;
        const BaseApp* m_app;

        void drawTaskBar();
        void drawLeftPanel();
        void drawPartsTab();
        void drawStagingTab();
    public:
        EditorGUI();
        EditorGUI(const BaseApp* app, const FontAtlas* atlas);
        ~EditorGUI();

        void setMasterPartList(const std::unordered_map<std::uint32_t, std::unique_ptr<BasePart>>* master_parts_list);
        void setSymmetrySides(int sides);
        void setRadialAlign(bool to_what);

        int getSymmetrySides() const;
        bool getRadialAlign() const;
        BasePart* getHighlightedPart() const;

        void onFramebufferSizeUpdate();
        void render();
        int update();
        void renderImGUI();

        const std::unique_ptr<BasePart>* getPickedObject() const;
};


#endif
