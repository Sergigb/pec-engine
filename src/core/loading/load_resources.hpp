#ifndef LOAD_RESOURCES_HPP
#define LOAD_RESOURCES_HPP

#include <memory>

#include "../../core/AssetManager.hpp" // ResourceMap typedef

class Resource;


void load_resources(ResourceMap& resource_map, const char* path="../data/resources.xml");


#endif


