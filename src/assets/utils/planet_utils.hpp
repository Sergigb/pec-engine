#ifndef PLANET_UTILS_HPP
#define PLANET_UTILS_HPP

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "../Planet.hpp"


btVector3 reference_ellipse_to_xyz(btScalar latitude, btScalar longitude, btScalar radius);


#endif
