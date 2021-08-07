#include <stb/stb_image.h>
#include <thread>

#include "Planet.hpp"
#include "../core/common.hpp"


#define TEXTURE_LOCATION 0
#define ELEVATION_LOCATION 1


Planet::Planet(RenderContext* render_context){
    UNUSED(render_context);
}


Planet::~Planet(){
}


dmath::mat4& Planet::getTransform(){
    return m_planet_transform;
}


dmath::mat4 Planet::getTransform() const{
    return m_planet_transform;
}


void Planet::render(const dmath::vec3& cam_translation){
    UNUSED(cam_translation);
}

void Planet::render(const dmath::vec3& cam_translation, const dmath::mat4 transform){
    UNUSED(cam_translation);
    UNUSED(transform);
}

