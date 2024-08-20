#ifndef APP_HPP
#define APP_HPP

#include <memory>
#include <cstdint>

#include "core/BaseApp.hpp"


#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


class Object;
class FontAtlas;
class GameEditor;
class SimulationRenderer;
class GameSimulation;


class App : public BaseApp{
    private:
        std::unique_ptr<GameEditor> m_editor;
        std::unique_ptr<GameSimulation> m_simulation;

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        bool m_quit;

        void init();
        void terminate();
    public:
        App();
        App(int gl_width, int gl_height);
        ~App();

        void run();
};


#endif
