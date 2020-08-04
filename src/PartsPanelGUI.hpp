#ifndef PARTSPANELGUI_HPP
#define PARTSPANELGUI_HPP

#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "BasePart.hpp"
#include "FontAtlas.hpp"
#include "Text2D.hpp"
#include "maths_funcs.hpp"
#include "RenderContext.hpp"



class PartsPanelGUI{
    private:
        GLuint m_fb, m_tex;
        GLuint m_text_shader, m_gui_shader;
        float m_fb_width, m_fb_height; // this is the size of the PANEL fb
        GLuint m_vao, m_vbo_vert, m_vbo_tex, m_vbo_clr;
        GLuint m_projection_location;

        std::unique_ptr<Text2D> m_text;

        math::mat4 m_projection;

        const RenderContext* m_render_context;
        const std::map<int, std::unique_ptr<BasePart>>* m_master_parts_list;
        const FontAtlas* m_font_atlas;
    public:
        PartsPanelGUI(float fb_width, float fb_height, const FontAtlas* atlas, GLuint text_shader, const RenderContext* render_context);
        ~PartsPanelGUI();

        void setMasterPartList(const std::map<int, std::unique_ptr<BasePart>>* master_parts_list);
        void onFramebufferSizeUpdate(float fb_width, float fb_height);

        void bindTexture();
        void render();
        void update();

        // methods
};

#endif
