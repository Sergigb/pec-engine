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
class Physics;


/* Planetarium GUI class, more docs incoming maybe... */


class PlanetariumGUI : public BaseGUI{
    private:
        float m_fb_width, m_fb_height;
        bool m_fb_update;

        std::unique_ptr<Text2D> m_main_text;

        const PlanetarySystem* m_planetary_system;
        double m_delta_t;
        std::uint32_t m_selected_planet;

        const FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
        const Camera* m_camera;
        const Physics* m_physics;

        void updateSceneText();
    public:
        PlanetariumGUI(const FontAtlas* atlas, const RenderContext* render_context,
                       const Camera* camera, const Physics* physics);
        ~PlanetariumGUI();

        void setPlanetarySystem(const PlanetarySystem* planetary_system);
        void setSimulationDeltaT(double delta_t);
        void setSelectedPlanet(std::uint32_t planet_id);

        void onFramebufferSizeUpdate();
        void render();
        int update();
};


#endif
