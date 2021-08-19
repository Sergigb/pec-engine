#include "PlanetarySystem.hpp"



PlanetarySystem::PlanetarySystem(RenderContext* render_context){
    m_render_context = render_context;

    // somehow we need to init the planets
}


PlanetarySystem::~PlanetarySystem(){
}


planet_map& PlanetarySystem::getPlanets(){
    return m_planets;
}


const planet_map& PlanetarySystem::getPlanets() const{
    return m_planets;
}
