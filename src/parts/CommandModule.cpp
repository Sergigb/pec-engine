#include <iostream>

#include "CommandModule.hpp"

CommandModule::CommandModule(){}
CommandModule::CommandModule(const std::string& name, const std::string& full_name, double mass, unsigned int type, bool root) : BasePart(name, full_name, mass, type, root){}
CommandModule::~CommandModule(){}

void CommandModule::updateState(){
	
}


