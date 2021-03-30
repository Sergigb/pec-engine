#ifndef PLANET_UTILS_HPP
#define PLANET_UTILS_HPP

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "Planet.hpp"


void build_surface(struct planet_surface& surface);
void clean_side(struct surface_node& node, int num_levels);
void bind_loaded_textures(struct planet_surface& surface);
void bind_loaded_texture(struct surface_node& node, struct planet_surface& surface);
void async_texture_load(struct surface_node* node, struct planet_surface* surface);
void texture_free(struct planet_surface& surface);

btVector3 reference_ellipse_to_xyz(btScalar latitude, btScalar longitude, btScalar radius);


#endif
