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
    if(m_input->pressed_keys[GLFW_KEY_TAB] & INPUT_KEY_DOWN && 
       m_behaviour == PLAYER_BEHAVIOUR_SIMULATION && m_vessel){
        switchVessel();
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

