#ifndef LOAD_STAR_SYSTEM_HPP
#define LOAD_STAR_SYSTEM_HPP

#include "../../assets/PlanetarySystem.hpp"


struct planetary_system;

void load_star_system(struct planetary_system& system, const char* path="../data/star_system.xml");

#endif


