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
class AssetManager;

namespace math{
    struct mat4;
}


/* Planetarium GUI class, more docs incoming maybe... */


class PlanetariumGUI : public BaseGUI{
    private:
        float m_fb_width, m_fb_height;
        bool m_fb_update;

        std::unique_ptr<Text2D> m_main_text;

        double m_delta_t;
        std::uint32_t m_selected_planet;

        const FontAtlas* m_font_atlas;
        const RenderContext* m_render_context;
        const Camera* m_camera;
        const Physics* m_physics;
        const AssetManager* m_asset_manager;

        void updateSceneText();
        void updateVesselsText(const math::mat4& proj_mat, const math::mat4& view_mat);
        void updatePlanetsText(const math::mat4& proj_mat, const math::mat4& view_mat);
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
