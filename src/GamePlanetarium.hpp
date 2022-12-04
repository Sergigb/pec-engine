#ifndef GAME_PLANETARIUM_HPP
#define GAME_PLANETARIUM_HPP

#include <memory>

class BaseApp;
class PlanetariumGUI;
class FontAtlas;


#define PLANETARIUM_SCALE_FACTOR 1e10


class GamePlanetarium{
    private:
        std::unique_ptr<PlanetariumGUI> m_gui;
        std::uint32_t m_selected_planet;
        bool m_freecam;

        BaseApp* m_app;

        void updateInput();
        void updateCamera();
        void switchPlanet();
    public:
        GamePlanetarium(BaseApp* app, const FontAtlas* font_atlas);
        ~GamePlanetarium();

        void update();

        const PlanetariumGUI* getPlanetariumGUI() const;
};


#endif
