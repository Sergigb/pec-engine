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


Player::Player(Camera* camera, AssetManager* asset_manager, Input* input){
	m_camera = camera;
	m_vessel = nullptr;
	m_asset_manager = asset_manager;
	m_input = input;
}


Player::~Player(){
	m_vessel = nullptr;
}


void Player::update(){
	if(!m_vessel){
		m_camera->freeCameraUpdate();
	}
	else{
		btTransform root_transform;
		btVector3 root_origin;

		m_vessel->getRoot()->m_body->getMotionState()->getWorldTransform(root_transform); // in the future it should be the CoM
		root_origin = root_transform.getOrigin();
		m_camera->setCameraPosition(dmath::vec3(root_origin.getX(), root_origin.getY(), root_origin.getZ()));
		m_camera->orbitalCameraUpdate();
	}
	if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)) && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
		if(m_vessel){
			m_vessel = nullptr;
		}
		else{
			if(m_asset_manager->m_editor_vessels.size()){
				std::map<std::uint32_t, std::shared_ptr<Vessel>>::iterator it = m_asset_manager->m_editor_vessels.begin();
				m_vessel = it->second.get();
			}
		}
	}
}

