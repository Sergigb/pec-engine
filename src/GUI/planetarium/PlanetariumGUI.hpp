#ifndef PLANETARIUMGUI_HPP
#define PLANETARIUMGUI_HPP

#include <memory>
#include <vector>

#include "../BaseGUI.hpp"
#include "../Sprite.hpp"


/* Planetarium actions */
#define PLANETARIUM_ACTION_NONE 0


class FontAtlas;
class RenderContext;
class Text2D;
class PlanetarySystem;
class Camera;
class Physics;
class AssetManager;
class Planet;

namespace math{
    struct mat4;
}


struct planet_gui_data{
    const Planet* m_planet_data;
    Sprite m_planet_sprite;
    math::vec3 m_pos_screen;
    float m_screen_dist;

    planet_gui_data(const Planet* planet, const Sprite& planet_sprite):
            m_planet_sprite(planet_sprite), m_pos_screen(0.0f, 0.0f, 0.0f){
        m_planet_data = planet;
        m_screen_dist = 0.0;
    }
};


struct system_gui_data{
    std::vector<struct planet_gui_data> m_planets_data;
    const PlanetarySystem* m_planetary_system;

};


/* Planetarium GUI class, more docs incoming maybe... */


class PlanetariumGUI : public BaseGUI{
    private:
        float m_fb_width, m_fb_height;
        bool m_fb_update;

        std::unique_ptr<Text2D> m_main_text;

        double m_delta_t;
        std::uint32_t m_selected_planet;
        struct system_gui_data m_system_gui_data;

        const FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
        const Camera* m_camera;
        const Physics* m_physics;
        const AssetManager* m_asset_manager;

        void buildSystemGUIData();
        void updateSceneText(const math::mat4& proj_mat, const math::mat4& view_mat);
        void updateVesselsText(const math::mat4& proj_mat, const math::mat4& view_mat);
        void renderPlanets(const math::mat4& proj_mat, const math::mat4& view_mat);
    public:
        // the constructor has too many arguments, maybe it's better to make the base objects available
        // from BaseApp
        PlanetariumGUI(const FontAtlas* atlas, const RenderContext* render_context,
                       const Camera* camera, const Physics* physics, 
                       const AssetManager* asset_manager);
        ~PlanetariumGUI();

        void setSimulationDeltaT(double delta_t);
        void setSelectedPlanet(std::uint32_t planet_id);

        void onFramebufferSizeUpdate();
        void render();
        int update();
};


#endif
