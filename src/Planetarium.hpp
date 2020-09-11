#ifndef PLANETARIUM_HPP
#define PLANETARIUM_HPP

#include "BaseApp.hpp"


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
