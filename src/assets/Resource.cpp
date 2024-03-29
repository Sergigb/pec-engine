#include "Resource.hpp"

#include <iostream>

Resource::Resource(const std::string& name, const std::string& fancy_name, char rtype, char rstate, double density, double temperature) : 
                   m_resource_name(name), m_resource_fancy_name(fancy_name){
    m_resource_type = rtype;
    m_resource_state = rstate;
    m_density = density;
    m_temperature = temperature;
}


Resource::~Resource(){

}


void Resource::setId(std::uint32_t id){
    m_resource_id = id;
}


char Resource::getType() const{
    return m_resource_type;
}


char Resource::getState() const{
    return m_resource_state;
}


double Resource::getDensity() const{
    return m_density;
}


double Resource::getTemperature() const{
    return m_temperature;
}


const std::string& Resource::getName() const{
    return m_resource_name;
}


const std::string& Resource::getFancyName() const{
    return m_resource_fancy_name;
}


std::uint32_t Resource::getId() const{
    return m_resource_id;
}

