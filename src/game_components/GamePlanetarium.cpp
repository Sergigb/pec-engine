#include <algorithm>
#include <cmath>

#include "GamePlanetarium.hpp"
#include "../core/BaseApp.hpp"
#include "../core/Player.hpp"
#include "../core/RenderContext.hpp"
#include "../core/AssetManager.hpp"
#include "../core/Camera.hpp"
#include "../core/Input.hpp"
#include "../core/RenderContext.hpp"
#include "../assets/PlanetarySystem.hpp"
#include "../GUI/planetarium/PlanetariumGUI.hpp"
#include "../renderers/PlanetariumRenderer.hpp"


bool comparator(const Planet* a, const Planet* b){
    return a->getOrbitalData().a_0 < b->getOrbitalData().a_0;
}


GamePlanetarium::GamePlanetarium(BaseApp* app, const FontAtlas* font_atlas){
    m_app = app;
    m_freecam = true;
    m_selected_planet = 0;
    m_selected_planet_idx = 0;

    m_input = m_app->getInput();
    m_asset_manager = m_app->getAssetManager();
    m_camera = m_app->getCamera();
    m_render_context = m_app->getRenderContext();

    m_gui.reset(new PlanetariumGUI(font_atlas, m_app));
    m_render_context->setGUI(m_gui.get(), GUI_MODE_PLANETARIUM);
    m_gui->setSelectedPlanet(0);

    m_renderer.reset(new PlanetariumRenderer(m_app));
    m_render_context->setRenderer(m_renderer.get(), RENDER_PLANETARIUM);

    // ordered planet vector init
    const planet_map& planets = m_asset_manager->m_planetary_system.get()->getPlanets();
    planet_map::const_iterator it;

    for(it=planets.begin();it!=planets.end();it++){
        m_ordered_planets.emplace_back(it->second.get());
    }
    std::sort(m_ordered_planets.begin(), m_ordered_planets.end(), comparator);
}


GamePlanetarium::~GamePlanetarium(){

}


void GamePlanetarium::updateInput(){
    if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)) 
        && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
        m_freecam = !m_freecam;
        if(!m_selected_planet && !m_freecam)
            switchPlanet();
    }

    if(m_input->pressed_keys[GLFW_KEY_TAB] & INPUT_KEY_DOWN && !m_freecam){
        switchPlanet();
    }

    double scx, scy;
    float fade = 0.0; /// this should go somewhere else
    m_input->getScroll(scx, scy);
    if((scy) && !m_render_context->imGuiWantCaptureMouse()){
        double current_distance = m_camera->getOrbitalCamDistance(), increment;

        if(current_distance < 0.15){
            increment = -SIGN(scy) * (0.08 * current_distance);
            fade = 1.0;
            }
        else if(current_distance < 1){ // fade at this interval
            fade = 1.0 - ((current_distance - 0.15) / (1 - 0.15));
            increment = -SIGN(scy) * (0.1 * current_distance);
        }
        else{
            increment = -SIGN(scy) * std::min((std::pow(std::abs(current_distance), 2.0) / 100.0)
                                              + 0.5, 75.0);
            fade = 0.0;
        }
        if(current_distance + increment > 10000.0){
            increment = 10000.0;
            increment = 0.0;
        }
        else if(current_distance + increment < 1e7 / PLANETARIUM_SCALE_FACTOR){
            increment = 0.0;
            current_distance = 1e7 / PLANETARIUM_SCALE_FACTOR;
        }
        m_gui->setTargetFade(fade);
        m_renderer->setTargetFade(fade);
        m_camera->setOrbitalCamDistance(current_distance + increment); // we should check the camera mode
    }
}


void GamePlanetarium::updateCamera(){
    if(!m_selected_planet || m_freecam){
        m_camera->freeCameraUpdate();
    }
    else if(m_selected_planet && !m_freecam){
        m_camera->setCameraPosition(
            m_asset_manager->m_planetary_system->getPlanets().at(m_selected_planet)->getPosition()
            / PLANETARIUM_SCALE_FACTOR);
        m_camera->setOrbitalInclination(0.0, dmath::vec3(0.0, 0.0, 0.0));
        m_camera->orbitalCameraUpdate();
    }
}


void GamePlanetarium::switchPlanet(){
    if(!m_selected_planet){
        m_selected_planet_idx = 0;  // initialized to 0 but just to be sure
    }
    else{
        m_selected_planet_idx++;
        if(m_selected_planet_idx >= m_ordered_planets.size()){
            m_selected_planet_idx = 0;
        }
    }

    m_selected_planet = m_ordered_planets.at(m_selected_planet_idx)->getId();
}


void GamePlanetarium::update(){
    updateInput();

    m_app->getPlayer()->setSelectedPlanet(m_selected_planet);
    m_gui->setSelectedPlanet(m_selected_planet);
    m_gui->setFreecam(m_freecam);
    m_gui->update();
}
