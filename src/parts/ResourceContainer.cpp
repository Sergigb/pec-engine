#include <iostream>

#include "ResourceContainer.hpp"

ResourceContainer::ResourceContainer(){}
ResourceContainer::ResourceContainer(const std::string& name,  const std::string& full_name, double mass, unsigned int type, bool root) : BasePart(name, full_name, mass, type, root){}
ResourceContainer::~ResourceContainer(){}

void ResourceContainer::updateState(){
	
}
