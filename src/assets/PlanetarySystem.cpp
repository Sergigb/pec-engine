#include "PlanetarySystem.hpp"
#include "../core/AssetManagerInterface.hpp"



PlanetarySystem::PlanetarySystem(RenderContext* render_context){
    m_render_context = render_context;
}


PlanetarySystem::~PlanetarySystem(){
}


const planet_map& PlanetarySystem::getPlanets() const{
    return *m_planets.get();
}


planet_map& PlanetarySystem::getPlanets(){
    return *m_planets.get();
}


const star& PlanetarySystem::getStar() const{
    return m_system_star;
}


const std::string& PlanetarySystem::getSystemName() const{
    return m_system_name;
}


void PlanetarySystem::setPlanetMap(std::unique_ptr<planet_map>& map){
    m_planets = std::move(map);
}


void PlanetarySystem::setSystemName(const char* name){
    m_system_name = name;
}


void PlanetarySystem::setStar(star& system_star){
    /* We copy every field in here, but hey... */
    m_system_star = system_star;
}


void PlanetarySystem::updateOrbitalElements(const double cent_since_j2000){
    planet_map::iterator it;

    for(it=m_planets->begin();it!=m_planets->end();it++){
        it->second->updateOrbitalElements(cent_since_j2000);
    }
}


void PlanetarySystem::updateRenderBuffers(const double current_time){
    planet_map::iterator it;

    for(it=m_planets->begin();it!=m_planets->end();it++){
        it->second->updateRenderBuffers(current_time);
    }
}


void PlanetarySystem::renderOrbits() const{
    planet_map::iterator it;

    for(it=m_planets->begin();it!=m_planets->end();it++){
        it->second->renderOrbit();
    }
}


void PlanetarySystem::updateKinematics(){
    planet_map::iterator it;

    for(it=m_planets->begin();it!=m_planets->end();it++){
        it->second->updateKinematics();
    }
}

