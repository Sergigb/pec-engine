#include <iostream>

#include "App.hpp"
#include "GUI/FontAtlas.hpp"
#include "GUI/DebugOverlay.hpp"
#include "core/RenderContext.hpp"
#include "core/AssetManager.hpp"
#include "game_components/GameEditor.hpp"
#include "game_components/GameSimulation.hpp"
#include "core/Physics.hpp"
#include "core/WindowHandler.hpp"
#include "core/Input.hpp"
#include "core/Camera.hpp"
#include "core/Frustum.hpp"
#include "core/Player.hpp"
#include "core/log.hpp"
#include "core/timing.hpp"
#include "renderers/SimulationRenderer.hpp"


App::App() : BaseApp(){
    init();
}


App::App(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


void App::init(){
    m_quit = false;

    m_physics->initDynamicsWorld();
    m_render_context->setDebugDrawer();

    if(m_asset_manager->loadResources() == EXIT_FAILURE){
        std::cerr << "App::init: fatal - failed to load the resources,"
                     "check the xml file!" << std::endl;
        log("App::init: fatal - failed to load resources, check the xml file!");
        exit(EXIT_FAILURE);
    }
    
    m_asset_manager->loadParts(); // no error check yet

    if(m_asset_manager->loadStarSystem() == EXIT_FAILURE){
        std::cerr << "App::init: fatal - failed to load the star system,"
                     "check the xml file!" << std::endl;
        log("App::init: fatal - failed to load the star system, check the xml file!");
        exit(EXIT_FAILURE);
    }

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);

    m_render_context->setDefaultFontAtlas(m_def_font_atlas.get());

    m_editor.reset(new GameEditor(this, m_def_font_atlas.get()));
    m_simulation.reset(new GameSimulation(this, m_def_font_atlas.get()));
}



App::~App(){
}


void App::terminate(){
    m_window_handler->setWindowShouldClose();
    m_asset_manager->cleanup();
    m_physics->stopSimulation();
    m_render_context->stop();
    m_window_handler->terminate();
}


void App::run(){
    double delta_t_ms = (1. / 60.) * 1000000.;
    logic_timing timing;
    timing.delta_t = delta_t_ms;

    m_physics->startSimulation(10);
    m_render_context->start();

    int edit_exit_status = m_editor->start();
    // when we have a main menu this will do somethign else
    if(edit_exit_status == EXIT_MAIN_MENU){
        terminate();
        return;
    }

    m_simulation->setUpSimulation();

    m_simulation->start();


    terminate();
}
