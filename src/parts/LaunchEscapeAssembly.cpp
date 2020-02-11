#include <iostream>

#include "LaunchEscapeAssembly.hpp"

LaunchEscapeAssembly::LaunchEscapeAssembly(){}
LaunchEscapeAssembly::LaunchEscapeAssembly(const std::string& name,  const std::string& full_name, double mass, unsigned int type, bool root) : BasePart(name, full_name, mass, type, root){}
LaunchEscapeAssembly::~LaunchEscapeAssembly(){}

void LaunchEscapeAssembly::updateState(){
	
}


