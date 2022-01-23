#ifndef PLANETARIUM_HPP
#define PLANETARIUM_HPP

#include <GL/glew.h>
#include <memory>
#include <vector>

#include "core/BaseApp.hpp"


class FontAtlas;
class Text2D;
class Planet;
class PlanetariumGUI;


#define MAX_ITER 10
#define REAL_TIME_DT (1000. / 60.0) / 1000.0


class Planetarium : public BaseApp{
    private:
        double m_delta_t, m_seconds_since_j2000, m_dt_multiplier;
        std::unique_ptr<Text2D> m_text, m_text2;

        std::vector<const Planet*> m_bodies;
        uint m_pick;

        std::unique_ptr<PlanetariumGUI> m_planetarium_gui;

        void init();
        void logic();
        void updateSceneText();
        void initGl();
        void renderOrbits();
        void render();

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        void processInput();
    public:
        Planetarium();
        Planetarium(int gl_width, int gl_height);
        ~Planetarium();

        void run();
};

#endif
