#include "GamePlanetarium.hpp"
#include "core/BaseApp.hpp"
#include "core/Player.hpp"
#include "core/RenderContext.hpp"
#include "core/AssetManager.hpp"
#include "core/Camera.hpp"
#include "core/Input.hpp"
#include "assets/PlanetarySystem.hpp"
#include "GUI/planetarium/PlanetariumGUI.hpp"


GamePlanetarium::GamePlanetarium(BaseApp* app, const FontAtlas* font_atlas){
    m_app = app;
    m_freecam = true;
    m_selected_planet = 0;


    m_gui.reset(new PlanetariumGUI(font_atlas, m_app->m_render_context.get(),
                                   m_app->m_camera.get(), m_app->m_physics.get(),
                                   m_app->m_asset_manager.get()));
    m_app->m_render_context->setGUI(m_gui.get(), GUI_MODE_PLANETARIUM);
    m_gui->setSelectedPlanet(0);
}


GamePlanetarium::~GamePlanetarium(){

}


const PlanetariumGUI* GamePlanetarium::getPlanetariumGUI() const{
    return m_gui.get();
}

void GamePlanetarium::updateInput(){
    // create these as member variables of this class
    const Input* input = m_app->m_input.get();

    if((input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)) 
        && (input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
        m_freecam = !m_freecam;
        if(!m_selected_planet && !m_freecam)
            switchPlanet();
    }

    if(input->pressed_keys[GLFW_KEY_TAB] & INPUT_KEY_DOWN && !m_freecam){
        switchPlanet();
    }
}


void GamePlanetarium::updateCamera(){
    // create these as member variables of this class
    const AssetManager* asset_manager = m_app->m_asset_manager.get();
    Camera* camera = m_app->m_camera.get();

    if(!m_selected_planet || m_freecam){
        camera->freeCameraUpdate();
    }
    else if(m_selected_planet && !m_freecam){
        camera->setCameraPosition(
            asset_manager->m_planetary_system->getPlanets().at(m_selected_planet)->getPosition()
            / PLANETARIUM_SCALE_FACTOR);
        camera->setOrbitalInclination(0.0, dmath::vec3(0.0, 0.0, 0.0));
        camera->orbitalCameraUpdate();
    }
}


void GamePlanetarium::switchPlanet(){
    // not the proper way of switching, we need the GUI for that
    const AssetManager* asset_manager = m_app->m_asset_manager.get();
    planet_map::const_iterator it;
    const planet_map& planets = asset_manager->m_planetary_system->getPlanets();

    if(!m_selected_planet){
        it = planets.begin();
        m_selected_planet = it->first;
        return;
    }
    else{
        it = planets.find(m_selected_planet);
    }

    it++;
    if(it != planets.end()){
        m_selected_planet = it->first;
    }
    else{
        it = planets.begin();
        m_selected_planet = it->first;
    }
}


void GamePlanetarium::update(){
    updateCamera();
    updateInput();

    m_app->m_player->setSelectedPlanet(m_selected_planet);
    m_gui->setSelectedPlanet(m_selected_planet);
    m_gui->setFreecam(m_freecam);
    m_gui->update();
}
