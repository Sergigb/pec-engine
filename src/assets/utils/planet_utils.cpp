#include <sstream>

#include <stb/stb_image.h>

#include "planet_utils.hpp"


btVector3 reference_ellipse_to_xyz(btScalar latitude, btScalar longitude, btScalar radius){
    btVector3 loc;

    // Longitude seems to be off when it's not 0, the latitude seems to be right
    loc.setX(btCos(latitude) * btCos(longitude));
    loc.setY(btCos(latitude) * btSin(longitude));
    loc.setZ(-btSin(latitude));
    loc *= radius;

    return loc;
}

