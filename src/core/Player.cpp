#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "Player.hpp"
#include "Input.hpp"
#include "AssetManager.hpp"
#include "WindowHandler.hpp"
#include "maths_funcs.hpp"
#include "Camera.hpp"
#include "log.hpp"
#include "../assets/Vessel.hpp"
#include "../assets/BasePart.hpp"
#include "../assets/PlanetarySystem.hpp"


typedef std::unordered_map<std::uint32_t, std::shared_ptr<Vessel>>::iterator VesselIterator;


Player::Player(Camera* camera, AssetManager* asset_manager, const Input* input){
    m_camera = camera;
    m_vessel = nullptr;
    m_asset_manager = asset_manager;
    m_input = input;
    m_behaviour = PLAYER_BEHAVIOUR_NONE;
    m_orbital_cam_mode = ORBITAL_CAM_MODE_ORBIT;
    m_selected_planet = 0;
    m_planetarium_freecam = false;
    m_planetarium_scale_factor = PLANETARIUM_DEF_SCALE_FACTOR;
    m_camera->getCameraParams(m_planetarium_cam_params);
    m_camera->getCameraParams(m_simulation_cam_params);
    m_camera->getCameraParams(m_editor_cam_params);
}


Player::~Player(){
    m_vessel = nullptr;
}



void Player::updateEditor(){
    // camera
    if(!m_vessel){
        m_camera->freeCameraUpdate();
    }
    else{
        const btVector3& com = m_vessel->getCoM();
        m_camera->setCameraPosition(dmath::vec3(com.getX(), com.getY(), com.getZ()));
        setCamAxisRotation();
        m_camera->orbitalCameraUpdate();
    }

    // input
    if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT))
       && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
        setPlayerTarget();
    }
}


void Player::updateSimulation(){
    // camera
    if(m_behaviour & PLAYER_BEHAVIOUR_NONE || !m_vessel){
        m_camera->freeCameraUpdate();
    }
    else if(m_vessel){ // sanity check for vessel
        const btVector3& com = m_vessel->getCoM();
        m_camera->setCameraPosition(dmath::vec3(com.getX(), com.getY(), com.getZ()));
        setCamAxisRotation();
        m_camera->orbitalCameraUpdate();
    }

    // input
    if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)) 
       && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
        setPlayerTarget();
    }
    else if(m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN){
        m_orbital_cam_mode = m_orbital_cam_mode == ORBITAL_CAM_MODE_ORBIT ? 
                             ORBITAL_CAM_MODE_SURFACE : ORBITAL_CAM_MODE_ORBIT;
    }

    if(m_input->pressed_keys[GLFW_KEY_TAB] & INPUT_KEY_DOWN){
        switchVessel();
    }
}


void Player::updatePlanetarium(){
    // camera
    if(!m_selected_planet || m_planetarium_freecam){
        m_camera->freeCameraUpdate();
    }
    else if(m_selected_planet && !m_planetarium_freecam){
        m_camera->setCameraPosition(
            m_asset_manager->m_planetary_system->getPlanets().at(m_selected_planet)->getPosition()
            / m_planetarium_scale_factor);
        m_camera->setOrbitalInclination(0.0, dmath::vec3(0.0, 0.0, 0.0));
        m_camera->orbitalCameraUpdate();
    }

    //input
    if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)) 
        && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
        setPlayerTarget();
    }

    if(m_input->pressed_keys[GLFW_KEY_TAB] & INPUT_KEY_DOWN){
        switchPlanet();
    }
}


void Player::setCamAxisRotation(){
    if(m_orbital_cam_mode == ORBITAL_CAM_MODE_ORBIT){
        m_camera->setOrbitalInclination(0.0, dmath::vec3(0.0, 0.0, 0.0));
    }
    else{
        btVector3 com = m_vessel->getCoM();
        dmath::vec3 com_d(com.getX(), com.getY(), com.getZ()), planet_normal, axis, up = dmath::vec3(0.0, 1.0, 0.0);

        // In the future, the asset manager (or someone else) should return the center of the planet
        // that the vessel is currently inside of (inside the SOI), it should also be an interface to access
        // general information about the system and the planets (such as atmospheres). For now we
        // assume that the 0.0, 0.0, 0.0 is the center of the current planet, the earth
        dmath::vec3 planet_center(0.0, 0.0, 0.0);

        planet_normal = dmath::normalise(planet_center - com_d);
        double theta = acos(dmath::dot(planet_normal, up) / dmath::length(planet_normal));
        axis = dmath::normalise(dmath::cross(planet_normal, up)); // right vector

        m_camera->setOrbitalInclination(theta, axis);
    }
}


void Player::unsetVessel(){
    m_vessel->setPlayer(nullptr);
    m_vessel = nullptr;
    m_camera->restoreCamOrientation();
}


void Player::setPlayerTarget(){
    if(m_behaviour & PLAYER_BEHAVIOUR_EDITOR){
        if(m_vessel){
            unsetVessel();
        }
        else if(m_asset_manager->m_editor_vessel.get()){
            m_vessel = m_asset_manager->m_editor_vessel.get();
            m_vessel->setPlayer(this);
        }
    }
    else if(m_behaviour & PLAYER_BEHAVIOUR_SIMULATION){
        if(m_vessel)
            unsetVessel();
        else if(m_asset_manager->m_active_vessels.size()){
            VesselIterator it = m_asset_manager->m_active_vessels.begin();
            m_vessel = it->second.get(); // in the future we should pick the closest
            m_vessel->setPlayer(this);
        }
    }
    else if(m_behaviour & PLAYER_BEHAVIOUR_PLANETARIUM){
        togglePlanetariumFreecam();
    }
}


void Player::switchVessel(){
    VesselIterator it = m_asset_manager->m_active_vessels.find(m_vessel->getId());

    if(it == m_asset_manager->m_active_vessels.end()){
        std::cerr << "Player::switchVessel: can't iterate to the next vessel because the current vessel's \
                      id can't be found in m_active_vessels" << std::endl;
        log("Player::switchVessel: can't iterate to the next vessel because the current vessel's \
                      id can't be found in m_active_vessels");
        return;
    }
    it++;
    if(it != m_asset_manager->m_active_vessels.end()){
        m_vessel = it->second.get();
    }
    else{
        it = m_asset_manager->m_active_vessels.begin();
        m_vessel = it->second.get();
    }
}


void Player::switchPlanet(){
    // this is just randomly sorted by unordered_map and its hashing, so a better system is needed
    planet_map::const_iterator it;
    const planet_map& planets = m_asset_manager->m_planetary_system->getPlanets();

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


void Player::onVesselDestroy(){
    m_vessel = nullptr;
    m_camera->restoreCamOrientation();
}


void Player::setBehaviour(short behaviour){
    switch(m_behaviour){
        case PLAYER_BEHAVIOUR_NONE:
            break;
        case PLAYER_BEHAVIOUR_EDITOR:
            m_camera->getCameraParams(m_editor_cam_params);
            break;
        case PLAYER_BEHAVIOUR_SIMULATION:
            m_camera->getCameraParams(m_simulation_cam_params);
            break;
        case PLAYER_BEHAVIOUR_PLANETARIUM:
            m_camera->getCameraParams(m_planetarium_cam_params);
            break;
    }

    switch(behaviour){
        case PLAYER_BEHAVIOUR_NONE:
            break;
        case PLAYER_BEHAVIOUR_EDITOR:
            m_camera->recoverCameraParams(m_editor_cam_params);
            break;
        case PLAYER_BEHAVIOUR_SIMULATION:
            m_camera->recoverCameraParams(m_simulation_cam_params);
            break;
        case PLAYER_BEHAVIOUR_PLANETARIUM:
            m_camera->recoverCameraParams(m_planetarium_cam_params);
            if(!m_selected_planet){
                switchPlanet();
            }
            break;
        default:
            std::cerr << "Player::setBehaviour: invalid player behaviour value: " << behaviour
                      << std::endl;
            log("Player::setBehaviour: invalid player behaviour value: ", behaviour);
            return;
    }

    m_behaviour = behaviour;
}


Vessel* Player::getVessel() const{
    return m_vessel;
}


void Player::togglePlanetariumFreecam(){
    m_planetarium_freecam = !m_planetarium_freecam;
}


void Player::setSelectedPlanet(std::uint32_t planet_id){
    m_selected_planet = planet_id;
}


bool Player::getPlanetariumFreecam() const{
    return m_planetarium_freecam;
}


void Player::setPlanetariumScaleFactor(double factor){
    m_planetarium_scale_factor = factor;
}


short Player::getBehaviour() const{
    return m_behaviour;
}


std::uint32_t Player::getPlanetariumSelectedPlanet() const{
    return m_selected_planet;
}