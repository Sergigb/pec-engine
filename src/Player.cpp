#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Player.hpp"
#include "Input.hpp"
#include "AssetManager.hpp"


Player::Player(Camera* camera, AssetManager* asset_manager, Input* input){
	m_camera = camera;
	m_vessel = nullptr;
	m_asset_manager = asset_manager;
	m_input = input;
}


Player::~Player(){
	m_vessel = nullptr;
}


#include <iostream>
void Player::update(){
	if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)) && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
		if(m_asset_manager->m_editor_vessels.size() > 0){
			m_vessel = m_asset_manager->m_editor_vessels.at(0).get();
		}
	}
}

