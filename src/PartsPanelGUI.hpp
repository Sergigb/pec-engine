#ifndef PARTSPANELGUI_HPP
#define PARTSPANELGUI_HPP

#include <map>
#include <cstdlib>
#include <memory>
#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "BasePart.hpp"
#include "FontAtlas.hpp"
#include "Text2D.hpp"
#include "maths_funcs.hpp"
#include "RenderContext.hpp"
#include "Input.hpp"


#define ITEM_SEPARATION 20
#define ITEM_COLOR_1 0.2f
#define ITEM_COLOR_2 0.3f
#define ITEM_MOUSEOVER_NONE -999999999 // :)

#define PANEL_SCROLL_STEP 30
#define PANEL_SCROLL_MARGIN 25

#define PANEL_ACTION_NONE 0
#define PANEL_ACTION_PICK 1

class PartsPanelGUI{
    private:
        GLuint m_fb, m_tex;
        GLuint m_gui_shader, m_text_shader;
        float m_fb_width, m_fb_height; // this is the size of the PANEL fb
        GLuint m_vao, m_vbo_vert, m_vbo_tex, m_vbo_clr;
        GLuint m_num_vert;
        GLuint m_projection_location, m_text_projection_location, m_disp_location;

        std::unique_ptr<Text2D> m_text;

        math::mat4 m_projection;

        int m_item_mouseover, m_last_item_colored;
        math::vec2 m_panel_scroll;
        std::map<int, int> m_item_to_key; // from list item to part key
        const std::unique_ptr<BasePart>* m_picked_part;

        const RenderContext* m_render_context;
        const std::map<std::uint32_t, std::unique_ptr<BasePart>>* m_master_parts_list;
        const FontAtlas* m_font_atlas;
        const Input* m_input;

        void updateBuffers();
        void buttonMouseoverColor();
    public:
        PartsPanelGUI(float fb_width, float fb_height, const FontAtlas* atlas, const RenderContext* render_context, const Input* input);
        ~PartsPanelGUI();

        void setMasterPartList(const std::map<std::uint32_t, std::unique_ptr<BasePart>>* master_parts_list);
        void onFramebufferSizeUpdate(float fb_width, float fb_height);

        void bindTexture();
        void render();
        int update(float mouse_x, float mouse_y);

        const std::unique_ptr<BasePart>* getPickedObject() const;

        // methods
};

#endif
