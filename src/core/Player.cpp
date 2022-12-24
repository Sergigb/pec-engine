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



Player::Player(Camera* camera, AssetManager* asset_manager, const Input* input){
    m_camera = camera;
    m_vessel = nullptr;
    m_asset_manager = asset_manager;
    m_input = input;
    m_behaviour = PLAYER_BEHAVIOUR_NONE;
    m_orbital_cam_mode = ORBITAL_CAM_MODE_ORBIT;
    m_selected_planet = 0;
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
        //setCamAxisRotation();
        m_camera->orbitalCameraUpdate();
    }

    // input
    if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT))
       && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
        setPlayerTarget();
    }
}


void Player::unsetVessel(){
    m_vessel->setPlayer(nullptr);
    m_vessel = nullptr;
    m_camera->restoreCamOrientation();
}


void Player::setVessel(Vessel* vessel){
    m_vessel = vessel;
    m_vessel->setPlayer(this);
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


void Player::setSelectedPlanet(std::uint32_t planet_id){
    m_selected_planet = planet_id;
}


short Player::getBehaviour() const{
    return m_behaviour;
}


std::uint32_t Player::getPlanetariumSelectedPlanet() const{
    return m_selected_planet;
}
