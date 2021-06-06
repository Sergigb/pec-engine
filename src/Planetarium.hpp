#ifndef PLANETARIUM_HPP
#define PLANETARIUM_HPP

#include <GL/glew.h>
#include <memory>
#include <vector>

#include "core/BaseApp.hpp"
#include "core/loading/load_star_system.hpp" // planetary_system defined here, move later


#define SECS_FROM_UNIX_TO_J2000 946684800.0
#define SECONDS_IN_A_CENTURY 3155760000.0
#define AU_TO_METERS 149597900000.0


class FontAtlas;
class Text2D;


class Planetarium : public BaseApp{
    private:
        double m_delta_t, m_seconds_since_j2000;
        struct planetary_system m_system;
        std::unique_ptr<Text2D> m_text, m_text2;

        void init();
        void logic();
        void updateSceneText();

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
