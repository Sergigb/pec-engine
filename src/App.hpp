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


class App : public BaseApp{
    private:
        std::unique_ptr<GameEditor> m_editor;

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        // game simulation state (temporary here)
        bool m_quit;
        std::chrono::duration<double, std::micro> m_elapsed_time;

        void init();
        void logic();
        void processInput();
        void onRightMouseButton();
    public:
        App();
        App(int gl_width, int gl_height);
        ~App();

        void run();
};


#endif
