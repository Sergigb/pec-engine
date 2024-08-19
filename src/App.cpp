#include <iostream>

#include "App.hpp"
#include "GUI/FontAtlas.hpp"
#include "GUI/DebugOverlay.hpp"
#include "core/RenderContext.hpp"
#include "core/AssetManager.hpp"
#include "game_components/GameEditor.hpp"
#include "game_components/GamePlanetarium.hpp"
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
    m_planetarium.reset(new GamePlanetarium(this, m_def_font_atlas.get()));
    m_simulation.reset(new GameSimulation(this, m_def_font_atlas.get()));
}



App::~App(){
}

void App::wakePhysics(){
    std::unique_lock<std::mutex> lck2(m_thread_monitor.mtx_start);
    m_thread_monitor.worker_start = true;
    m_thread_monitor.cv_start.notify_all();
}


void App::waitPhysics(){
    std::unique_lock<std::mutex> lck(m_thread_monitor.mtx_end);
    while(!m_thread_monitor.worker_ended){
        m_thread_monitor.cv_end.wait(lck);
    }
    m_thread_monitor.worker_ended = false;
}


void App::setUpSimulation(){
    m_physics->pauseSimulation(false);
    m_gui_mode = GUI_MODE_NONE;
    m_render_state = RENDER_SIMULATION;
    m_player->setBehaviour(PLAYER_BEHAVIOUR_SIMULATION);
    m_camera->setCameraPosition(dmath::vec3(9300000.0, 0.0, 0.0));
    m_camera->setSpeed(630000.0f);
    m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));
}


void App::logic(){
    if(m_player->getBehaviour() & PLAYER_BEHAVIOUR_SIMULATION)
        m_simulation->update();
    else
        m_planetarium->update();
    processInput();
    m_asset_manager->updateVessels();
}


void App::terminate(){
    m_window_handler->setWindowShouldClose();
    m_asset_manager->cleanup();
    m_physics->stopSimulation();
    m_render_context->stop();
    m_window_handler->terminate();
}


void App::synchPreStep(){
    m_asset_manager->processCommandBuffers(false);
    m_input->update();
    m_window_handler->update();
    m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
}


void App::synchPostStep(){
    m_asset_manager->updateCoMs();
    // camera updates need synch calls
    if(m_player->getBehaviour() & PLAYER_BEHAVIOUR_SIMULATION)
        m_simulation->updateCamera();
    else
        m_planetarium->updateCamera();
    m_asset_manager->updateBuffers();
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

    m_simulation->onStateChange();

    setUpSimulation();

    while(!m_quit){
        timing.register_tp(TP_LOGIC_START);

        synchPreStep();
        wakePhysics();
        logic();

        m_render_context->getDebugOverlay()->setLogicTimes(timing);

        waitPhysics();
        synchPostStep();

        timing.register_tp(TP_LOGIC_END);
        timing.update();

        if(timing.current_sleep > 0.0){
            std::this_thread::sleep_for(duration(timing.current_sleep));
        }
    }

    terminate();
}


void App::processInput(){
    if(m_render_context->imGuiWantCaptureKeyboard()){
        return;
    }

    if(m_input->pressed_keys[GLFW_KEY_ESCAPE] == INPUT_KEY_DOWN){
        m_quit = true;
    }

    if(m_input->pressed_keys[GLFW_KEY_M] == INPUT_KEY_DOWN){
        if(m_player->getBehaviour() & PLAYER_BEHAVIOUR_SIMULATION){
            setRenderState(RENDER_PLANETARIUM);
            setGUIMode(GUI_MODE_PLANETARIUM);
            m_player->setBehaviour(PLAYER_BEHAVIOUR_PLANETARIUM);
        }
        else{
            setRenderState(RENDER_SIMULATION);
            setGUIMode(GUI_MODE_NONE);
            m_player->setBehaviour(PLAYER_BEHAVIOUR_SIMULATION);
        }
    }

    if(m_input->pressed_keys[GLFW_KEY_F12] == INPUT_KEY_DOWN){
        m_render_context->toggleDebugOverlay();
    }

    if(m_input->pressed_keys[GLFW_KEY_F11] == INPUT_KEY_DOWN){
        m_render_context->toggleDebugDraw();
    }

    if(m_input->pressed_keys[GLFW_KEY_F10] == INPUT_KEY_DOWN){
        m_render_context->reloadShaders();
        m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));
    }
}