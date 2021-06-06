#ifndef PLANETARIUM_HPP
#define PLANETARIUM_HPP

#include <GL/glew.h>
#include <memory>
#include <vector>

#include "core/BaseApp.hpp"


#define SECS_FROM_UNIX_TO_J2000 946684800.0
#define SECONDS_IN_A_CENTURY 3155760000.0
#define AU_TO_METERS 149597900000.0


class FontAtlas;


class Planetarium : public BaseApp{
    private:
        void init();

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;


    public:
        Planetarium();
        Planetarium(int gl_width, int gl_height);
        ~Planetarium();

        void run();
};


#endif
