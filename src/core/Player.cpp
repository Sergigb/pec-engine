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


typedef std::unordered_map<std::uint32_t, std::shared_ptr<Vessel>>::iterator VesselIterator;


Player::Player(Camera* camera, AssetManager* asset_manager, const Input* input){
    m_camera = camera;
    m_vessel = nullptr;
    m_asset_manager = asset_manager;
    m_input = input;
    m_behaviour = PLAYER_BEHAVIOUR_NONE;
    m_orbital_cam_mode = ORBITAL_CAM_MODE_ORBIT;
}


Player::~Player(){
    m_vessel = nullptr;
}


void Player::update(){
    if(!m_vessel){
        m_camera->freeCameraUpdate();
    }
    else{
        btVector3 com = m_vessel->getCoM();
        m_camera->setCameraPosition(dmath::vec3(com.getX(), com.getY(), com.getZ()));
        m_camera->orbitalCameraUpdate();
    }

    if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)) 
        && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
        setCamMode();
    }
    else if(m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN && m_vessel){
        if(m_orbital_cam_mode == ORBITAL_CAM_MODE_ORBIT){
            m_orbital_cam_mode = ORBITAL_CAM_MODE_SURFACE;
        }
        else{
            m_orbital_cam_mode = ORBITAL_CAM_MODE_ORBIT;
        }
    }

    if(m_vessel){
        setCamUpVector();    
    }

    if(m_input->pressed_keys[GLFW_KEY_TAB] & INPUT_KEY_DOWN && 
       m_behaviour == PLAYER_BEHAVIOUR_SIMULATION && m_vessel){
        switchVessel();
    }
}


void Player::setCamUpVector(){
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
        double theta = M_PI + acos(dmath::dot(planet_normal, up) / (dmath::length(planet_normal) * 1.0));
        axis = dmath::normalise(dmath::cross(planet_normal, up)); // right vector

        m_camera->setOrbitalInclination(theta, axis);
    }
}


void Player::setCamMode(){
    if(m_vessel){
        m_vessel->setPlayer(nullptr);
        m_vessel = nullptr;
        m_camera->restoreCamOrientation();
    }
    else{
        if(m_behaviour == PLAYER_BEHAVIOUR_EDITOR && m_asset_manager->m_editor_vessel.get()){
            m_vessel = m_asset_manager->m_editor_vessel.get();
            m_vessel->setPlayer(this);
        }
        else if(m_behaviour == PLAYER_BEHAVIOUR_SIMULATION && m_asset_manager->m_active_vessels.size()){
            VesselIterator it = m_asset_manager->m_active_vessels.begin();
            m_vessel = it->second.get(); // greedily pick the first vessel
        }
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


void Player::onVesselDestroy(){
    m_vessel = nullptr;
    m_camera->restoreCamOrientation();
}


void Player::setBehaviour(short behaviour){
    m_behaviour = behaviour;
}


Vessel* Player::getVessel() const{
    return m_vessel;
}

