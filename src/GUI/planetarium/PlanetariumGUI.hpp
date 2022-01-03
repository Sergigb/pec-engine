#ifndef PLANETARIUMGUI_HPP
#define PLANETARIUMGUI_HPP

#include <memory>

#include "../BaseGUI.hpp"


/* Planetarium actions */
#define PLANETARIUM_ACTION_NONE 0


class FontAtlas;
class RenderContext;
class Text2D;
class PlanetarySystem;
class Camera;


/* Planetarium GUI class, more docs incoming maybe... */


class PlanetariumGUI : public BaseGUI{
    private:
        float m_fb_width, m_fb_height;
        bool m_fb_update;

        std::unique_ptr<Text2D> m_main_text;

        const PlanetarySystem* m_planetary_system;
        double m_delta_t;

        const FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
        const Camera* m_camera;

        void updateSceneText();
    public:
        PlanetariumGUI(const FontAtlas* atlas, const RenderContext* render_context, const Camera* camera);
        ~PlanetariumGUI();

        void setPlanetarySystem(const PlanetarySystem* planetary_system);
        void setSimulationDeltaT(double delta_t);

        void onFramebufferSizeUpdate();
        void render();
        int update();
};


#endif
