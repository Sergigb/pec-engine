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
#define PLANETARIUM_ACTION_SET_VELOCITY 1
#define PLANETARIUM_ACTION_SET_POSITION 2
#define PLANETARIUM_ACTION_SET_ORBIT 3


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

    planet_gui_data(){
        m_planet_data = nullptr;
    }

    planet_gui_data(const Planet* planet, Sprite&& planet_sprite):
            m_planet_sprite(std::move(planet_sprite)), m_pos_screen(0.0f, 0.0f, 0.0f){
        m_planet_data = planet;
        m_screen_dist = 0.0;
    }

    planet_gui_data& operator=(planet_gui_data&& data){
        m_planet_sprite = std::move(data.m_planet_sprite);
        m_pos_screen = data.m_pos_screen;
        m_planet_data = data.m_planet_data;
        m_screen_dist = data.m_screen_dist;
        return *this;
    }

    planet_gui_data(planet_gui_data&& data) noexcept :
            m_planet_sprite(std::move(data.m_planet_sprite)),
            m_pos_screen(data.m_pos_screen){
        m_planet_data = data.m_planet_data;
        m_screen_dist = data.m_screen_dist;
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


struct cheat_orbit{
    struct orbital_data cheat_orbit_params;
    std::uint32_t body_target;
    bool match_frame;

    cheat_orbit(){
        body_target = 0;
        match_frame = true;
        // set to earth because why not
        cheat_orbit_params.a_0 = 149600000000.0;
        cheat_orbit_params.e_0 = 0.01671123 * ONE_DEG_IN_RAD;
        cheat_orbit_params.i_0 = -0.00001531 * ONE_DEG_IN_RAD;
        cheat_orbit_params.L_0 = 100.46457166 * ONE_DEG_IN_RAD;
        cheat_orbit_params.W_0 = 0.0 * ONE_DEG_IN_RAD;
        cheat_orbit_params.p_0 = 102.93768193 * ONE_DEG_IN_RAD;
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

        // gui
        bool m_show_predictor_settings, m_show_cheats;
        int m_action;
        double m_cheat_vel_x, m_cheat_vel_y, m_cheat_vel_z; 
        double m_cheat_pos_x, m_cheat_pos_y, m_cheat_pos_z;
        struct cheat_orbit m_cheat_orbit;    

        const FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
        const Camera* m_camera;
        const Physics* m_physics;
        const AssetManager* m_asset_manager;
        const BaseApp* m_app;

        void buildSystemGUIData();
        void updateSceneText(const math::mat4& proj_mat, const math::mat4& view_mat);
        void updateVesselsText(const math::mat4& proj_mat, const math::mat4& view_mat);
        void renderPlanets(const math::mat4& proj_mat, const math::mat4& view_mat);
        void showCheatsMenu();
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
        void renderImGUI();

        const btVector3 getCheatVelocity() const;
        const btVector3 getCheatPosition() const;
        const struct cheat_orbit getCheatOrbitParameters() const;

        // move this shit to a struct with "predictor config parameters"
        int m_predictor_steps;
        float m_predictor_period;
};


#endif
