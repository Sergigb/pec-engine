#include <iostream>

#include "App.hpp"
#include "GUI/FontAtlas.hpp"
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
#include "assets/Planet.hpp"
#include "assets/BasePart.hpp"
#include "assets/Vessel.hpp"
#include "assets/Model.hpp"
#include "assets/Kinematic.hpp"
#include "assets/utils/planet_utils.hpp"
#include "assets/PlanetarySystem.hpp"
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

    m_asset_manager->loadResources();
    m_asset_manager->loadParts();
    m_asset_manager->loadStarSystem();

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
        m_planetarium->updateCamera();
    else
        m_simulation->updateCamera();
    m_asset_manager->updateBuffers();
}


void App::run(){
    m_physics->startSimulation(10);
    m_render_context->start();

    m_editor->start();
    m_simulation->onStateChange();

    std::chrono::steady_clock::time_point loop_start_load;
    std::chrono::steady_clock::time_point previous_loop_start_load = std::chrono::steady_clock::now();;
    std::chrono::steady_clock::time_point loop_end_load;
    double delta_t = (1. / 60.) * 1000000., accumulated_load = 0.0, accumulated_sleep = 0.0, average_load = 0.0, average_sleep = 0.0;
    int ticks_since_last_update = 0;

    setUpSimulation();

    while(!m_quit){
        loop_start_load = std::chrono::steady_clock::now();

        synchPreStep();
        wakePhysics();
        logic();

        m_render_context->setDebugOverlayTimes(m_physics->getAverageLoadTime(), average_load, average_sleep);
        
        m_elapsed_time += loop_start_load - previous_loop_start_load;
        previous_loop_start_load = loop_start_load;
        
        if(ticks_since_last_update == 60){
            ticks_since_last_update = 0;
            average_load = accumulated_load / 60000.0;
            average_sleep = accumulated_sleep / 60000.0;
            accumulated_load = 0;
            accumulated_sleep = 0;
        }
        ticks_since_last_update++;

        waitPhysics();
        synchPostStep();

        loop_end_load = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::micro> load_time = loop_end_load - loop_start_load;
        accumulated_load += load_time.count();
        accumulated_sleep += delta_t - load_time.count();

        if(load_time.count() < delta_t){
            std::chrono::duration<double, std::micro> delta_ms(delta_t - load_time.count());
            std::this_thread::sleep_for(delta_ms);
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