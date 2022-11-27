#ifndef GAME_PLANETARIUM_HPP
#define GAME_PLANETARIUM_HPP

#include <memory>

class BaseApp;
class PlanetariumGUI;
class FontAtlas;


class GamePlanetarium{
    private:
        std::unique_ptr<PlanetariumGUI> m_gui;

        BaseApp* m_app;
    public:
        GamePlanetarium(BaseApp* app, const FontAtlas* font_atlas);
        ~GamePlanetarium();

        void update();

        const PlanetariumGUI* getPlanetariumGUI() const;
};


#endif
