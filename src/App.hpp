#ifndef APP_HPP
#define APP_HPP

#include <memory>
#include <cstdint>

#include "BaseApp.hpp"


#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


class Object;
class FontAtlas;
class GameEditor;


#define MAX_SYMMETRY_SIDES 8
#define SIDE_ANGLE_STEP double(M_PI / MAX_SYMMETRY_SIDES)


class App : public BaseApp{
    private:
        std::unique_ptr<GameEditor> m_editor;

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        void init();
    public:
        App();
        App(int gl_width, int gl_height);
        ~App();

        void run();
};


#endif
