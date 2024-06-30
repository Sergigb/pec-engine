#ifndef PLANETARIUMGUI_HPP
#define PLANETARIUMGUI_HPP

#include <memory>
#include <vector>
#include <unordered_map>

#include "../BaseGUI.hpp"
#include "../Sprite.hpp"
#include "../../assets/Planet.hpp"


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
class BaseApp;

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
    std::unordered_map<std::uint32_t, uint> m_id_to_index;

    void update_id_map(){
        m_id_to_index.clear();
        for(uint i=0; i < m_planets_data.size(); i++){
            std::uint32_t id = m_planets_data.at(i).m_planet_data->getId();
            m_id_to_index.insert({id, i});
        }
    }
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
        bool m_freecam;
        float m_target_fade = 0.0;

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
        PlanetariumGUI(const FontAtlas* atlas, const BaseApp* app);
        ~PlanetariumGUI();

        void setSimulationDeltaT(double delta_t);
        void setSelectedPlanet(std::uint32_t planet_id);
        void setFreecam(bool freecam);
        void setTargetFade(float value);

        void onFramebufferSizeUpdate();
        void render();
        int update();
};


#endif
