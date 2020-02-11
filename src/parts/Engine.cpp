#include <iostream>

#include "Engine.hpp"

Engine::Engine(){}
Engine::Engine(const std::string& name,  const std::string& full_name, double mass, unsigned int type, bool root) : BasePart(name, full_name, mass, type, root){}
Engine::~Engine(){}

void Engine::updateState(){
	
}


