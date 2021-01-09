#ifndef PLANET_UTILS_HPP
#define PLANET_UTILS_HPP

#include "Planet.hpp"


namespace plutils{
    void build_surface(struct planet_surface& surface);
    void clean_side(struct surface_node& node, int num_levels);
}


#endif
