#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "Player.hpp"
#include "Input.hpp"
#include "AssetManager.hpp"
#include "WindowHandler.hpp"
#include "Vessel.hpp"
#include "maths_funcs.hpp"
#include "Camera.hpp"
#include "BasePart.hpp"


Player::Player(Camera* camera, AssetManager* asset_manager, Input* input){
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
        if(m_vessel){
            m_vessel->setPlayer(nullptr);
            m_vessel = nullptr;
            m_camera->restoreCamOrientation();
        }
        else{
            if(m_asset_manager->m_editor_vessel.get()){
                m_vessel = m_asset_manager->m_editor_vessel.get();
                m_vessel->setPlayer(this);
            }
        }
    }
}


void Player::onVesselDestroy(){
    m_vessel = nullptr;
    m_camera->restoreCamOrientation();
}


void setBehaviour(){

}

