#include <unordered_map>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <algorithm>

#include "Planetarium.hpp"
#include "core/log.hpp"
#include "core/utils/utils.hpp"
#include "core/maths_funcs.hpp"
#include "core/RenderContext.hpp"
#include "core/Camera.hpp"
#include "core/Input.hpp"
#include "core/WindowHandler.hpp"
#include "core/Player.hpp"
#include "core/Physics.hpp"
#include "core/Frustum.hpp"
#include "core/AssetManager.hpp"
#include "GUI/FontAtlas.hpp"
#include "GUI/Text2D.hpp"
#include "GUI/planetarium/PlanetariumGUI.hpp"
#include "assets/PlanetarySystem.hpp"
#include "assets/Planet.hpp"


Planetarium::Planetarium() : BaseApp(){
    init();
}


Planetarium::Planetarium(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


bool comparator(const Planet* a, const Planet* b){
    return a->getOrbitalData().a_0 < b->getOrbitalData().a_0;
}


void Planetarium::init(){
    m_window_handler->setWindowTitle("Planetarium");
    m_seconds_since_j2000 = 0.0;

    m_asset_manager->loadStarSystem();

    m_delta_t = (1000. / 60.0) / 1000.0;  // in ms

    planet_map::const_iterator it;
    const planet_map& planets = m_asset_manager->m_planetary_system->getPlanets();

    for(it=planets.begin();it!=planets.end();it++)
        m_bodies.emplace_back(it->second.get());
    std::sort(m_bodies.begin(), m_bodies.end(), comparator);
    m_pick = 2;
    m_player->setSelectedPlanet(m_bodies.at(m_pick)->getId());

    initGl();

    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);

    int fb_width, fb_height;
    m_window_handler->getFramebufferSize(fb_width, fb_height);

    m_text.reset(new Text2D(fb_width, fb_height, {0.f, 1.f, 1.f}, 
                            m_def_font_atlas.get(), m_render_context.get()));
    m_text2.reset(new Text2D(fb_width, fb_height, {0.f, 1.f, 0.f}, 
                            m_def_font_atlas.get(), m_render_context.get()));

    m_planetarium_gui.reset(new PlanetariumGUI(m_def_font_atlas.get(), m_render_context.get(),
                                               m_camera.get(), m_physics.get(),
                                               m_asset_manager.get()));
}


Planetarium::~Planetarium(){
}


GLuint color_location, proj_location, view_location; // change me :)

void Planetarium::initGl(){
    color_location = m_render_context->getUniformLocation(SHADER_DEBUG, "line_color");
    proj_location = m_render_context->getUniformLocation(SHADER_DEBUG, "proj");
    view_location = m_render_context->getUniformLocation(SHADER_DEBUG, "view");
}


void Planetarium::renderOrbits(){
    planet_map::const_iterator it;
    const planet_map& planets = m_asset_manager->m_planetary_system->getPlanets();

    m_render_context->useProgram(SHADER_DEBUG);
    glUniformMatrix4fv(view_location, 1, GL_FALSE, m_camera->getViewMatrix().m);
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, m_camera->getProjMatrix().m);

    for(it=planets.begin();it!=planets.end();it++){
        Planet* current = it->second.get();

        if(current == m_bodies.at(m_pick))
            glUniform3f(color_location, 0.0, 1.0, 0.0);
        else
            glUniform3f(color_location, 1.0, 0.0, 0.0);

        current->renderOrbit();
    }
}


void Planetarium::processInput(){
    double scx, scy;
    m_input->getScroll(scx, scy);

    if(m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & INPUT_KEY_REPEAT)
        m_delta_t *= scy == 0.f ? 1.f : (scy < 0.f ? .1f : 10.f);
    
    else{
        if(m_player->getPlanetariumFreecam())
            m_camera->setSpeed(m_camera->getSpeed() * (scy == 0.f ? 1.f : (scy < 0.f ? .1f : 10.f)));
        else
            m_camera->incrementOrbitalCamDistance(-scy * 5.0);
    }

    if(m_input->pressed_keys[GLFW_KEY_R] & INPUT_KEY_RELEASE){
        m_render_context->reloadShaders();
        std::cout << "Shaders reloaded" << std::endl;
    }

    if(m_input->pressed_keys[GLFW_KEY_ESCAPE] & INPUT_KEY_RELEASE){
        glfwSetWindowShouldClose(m_window_handler->getWindow(), GLFW_TRUE);
    }

    if(m_input->pressed_keys[GLFW_KEY_F12] & INPUT_KEY_RELEASE){
        m_render_context->toggleDebugOverlay();
    }

    if(m_input->pressed_keys[GLFW_KEY_TAB] & INPUT_KEY_DOWN){
        m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & INPUT_KEY_REPEAT ? m_pick-- : m_pick++;
        if(m_pick >= m_bodies.size())
            m_pick = 0;
        m_player->setSelectedPlanet(m_bodies.at(m_pick)->getId());
    }
}


void Planetarium::logic(){
    double centuries_since_j2000 = 0.0;

    processInput();

    centuries_since_j2000 = m_seconds_since_j2000 / SECONDS_IN_A_CENTURY;
    m_asset_manager->m_planetary_system->updateOrbitalElements(centuries_since_j2000);

    m_planetarium_gui->setSelectedPlanet(m_bodies.at(m_pick)->getId());
    m_planetarium_gui->onFramebufferSizeUpdate(); // it's nasty to do this evey frame...
    m_planetarium_gui->update();

    m_seconds_since_j2000 += m_delta_t;

    m_asset_manager->m_planetary_system->updateRenderBuffers(centuries_since_j2000);
    renderOrbits();
    m_planetarium_gui->render();
}


void Planetarium::run(){
    m_camera->setCameraPosition(dmath::vec3(500.0, 1000.0, 0.0));
    m_camera->setSpeed(50.0f);
    m_camera->createProjMat(.001, 100000.0, 67.0, 1.0);
    m_player->setBehaviour(PLAYER_BEHAVIOUR_PLANETARIUM);

    //glDisable(GL_CULL_FACE);

    m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));

    while(!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
        m_player->updatePlanetarium();

        m_render_context->contextUpdatePlanetRenderer(); // works in here too :)
        glClearColor(0.f, 0.f, 0.f, 0.f); // loook at the todo list
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        logic();

        glfwSwapBuffers(m_window_handler->getWindow());
    }

    m_window_handler->terminate();
}

