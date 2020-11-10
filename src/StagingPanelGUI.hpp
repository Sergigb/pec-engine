#ifndef STAGINGPANELGUI_HPP
#define STAGINGPANELGUI_HPP

#include <map>
#include <memory>
#include <cstdint>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "maths_funcs.hpp"


class FontAtlas;
class Text2D;
class RenderContext;
class Input;



class StagingPanelGUI{
    private:
        GLuint m_fb, m_tex;
        GLuint m_projection_location, m_text_projection_location, m_disp_location;

        math::mat4 m_projection;
        float m_fb_width, m_fb_height;

        std::unique_ptr<Text2D> m_text;

        const RenderContext* m_render_context;
        const FontAtlas* m_font_atlas;
        const Input* m_input;

        void updateBuffers();
    public:
        StagingPanelGUI(float fb_width, float fb_height, const FontAtlas* atlas, const RenderContext* render_context, const Input* input);
        ~StagingPanelGUI();

        void onFramebufferSizeUpdate(float fb_width, float fb_height);

        void bindTexture();
        void render();
        //int update(float mouse_x, float mouse_y);

};

#endif