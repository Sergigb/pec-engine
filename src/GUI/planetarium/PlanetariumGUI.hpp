#ifndef PLANETARIUMGUI_HPP
#define PLANETARIUMGUI_HPP

#include <memory>

#include "../BaseGUI.hpp"


/* Planetarium actions */
#define PLANETARIUM_ACTION_NONE 0


class FontAtlas;
class RenderContext;
class Text2D;


/* Planetarium GUI class, more docs incoming maybe... */


class PlanetariumGUI : public BaseGUI{
    private:
        float m_fb_width, m_fb_height;
        bool m_fb_update;

        std::unique_ptr<Text2D> m_main_text;

        const FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
    public:
        PlanetariumGUI(const FontAtlas* atlas, const RenderContext* render_context);
        ~PlanetariumGUI();

        void onFramebufferSizeUpdate();
        void render();
        int update();
};


#endif
