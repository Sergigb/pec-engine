#include <iostream>

#include "ServiceModule.hpp"

ServiceModule::ServiceModule(){}
ServiceModule::ServiceModule(const std::string& name, const std::string& full_name, double mass, unsigned int type, bool root) : BasePart(name, full_name, mass, type, root){}
ServiceModule::~ServiceModule(){}

void ServiceModule::updateState(){
	
}


