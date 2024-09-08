#ifndef LOAD_PARTS_HPP
#define LOAD_PARTS_HPP

#include "../../core/AssetManager.hpp"

class BaseApp;

#define CYLINDER_SHAPE 1

int load_parts(BasePartMap& part_map, const char* path, BaseApp* app);


#endif