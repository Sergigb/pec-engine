#ifndef GAME_PLANETARIUM_HPP
#define GAME_PLANETARIUM_HPP

#include <memory>
#include <vector>
#include <stdint.h>

class BaseApp;
class PlanetariumGUI;
class FontAtlas;
class Planet;
class Input;
class AssetManager;
class Camera;
class PlanetariumRenderer;
class RenderContext;


#define PLANETARIUM_SCALE_FACTOR 1e8


class GamePlanetarium{
    private:
        std::unique_ptr<PlanetariumGUI> m_gui;
        std::unique_ptr<PlanetariumRenderer> m_renderer;

        uint32_t m_selected_planet;
        uint m_selected_planet_idx;
        bool m_freecam;
        std::vector<const Planet*> m_ordered_planets;

        BaseApp* m_app;
        const Input* m_input;
        const AssetManager* m_asset_manager;
        Camera* m_camera;
        RenderContext* m_render_context;

        void updateInput();
        void switchPlanet();
    public:
        GamePlanetarium(BaseApp* app, const FontAtlas* font_atlas);
        ~GamePlanetarium();

        void update();
        void updateCamera();
};


#endif
