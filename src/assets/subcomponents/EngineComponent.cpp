#include <cassert>

#include "EngineComponent.hpp"
#include "../Vessel.hpp"
#include "../BasePart.hpp"


EngineComponent::EngineComponent(BasePart* owner_part, const btVector3& origin,
                                 const btVector3& orientation, double max_avg_thrust)
                                 : m_local_origin(origin), m_local_orientation(orientation) {
    assert(owner_part);

    m_local_orientation = m_local_orientation.normalize();

    m_max_angle_yaw = 0.0;
    m_max_angle_pitch = 0.0;
    m_yaw = false;
    m_pitch = false;
    m_max_avg_thrust = max_avg_thrust;
    m_status = ENGINE_STATUS_OFF;
    m_can_be_stopped = true;
    m_throttle = 1.0;
    m_request_owner = false;

    m_owner_part = owner_part;

}


EngineComponent::EngineComponent(){
    m_local_origin = btVector3(0.0, 0.0, 0.0);
    m_owner_part = nullptr;
    m_local_orientation = btVector3(0., -1., 0.);

    m_max_angle_yaw = 0.0;
    m_max_angle_pitch = 0.0;
    m_yaw = false;
    m_pitch = false;
    m_status = ENGINE_STATUS_OFF;
    m_can_be_stopped = true;
    m_throttle = 1.0;
    m_request_owner = false;
}


const btVector3& EngineComponent::getThrustOrigin() const{
    return m_local_origin;
}


const btVector3& EngineComponent::getThrustOrientation() const{
    return m_local_orientation;
}


const btMatrix3x3 EngineComponent::getDeflectionBasis() const{
    btMatrix3x3 deflect_basis;
    const Vessel* vessel = m_owner_part->getVessel();

    deflect_basis.setEulerZYX(vessel->getYaw() * m_max_angle_yaw, 0.0,
                              vessel->getPitch() * m_max_angle_pitch);

    return deflect_basis;
}


const btVector3 EngineComponent::getDeflectedThrustDirection() const{
    const btMatrix3x3& deflect_basis = getDeflectionBasis();
    const btVector3 deflected_thrust = deflect_basis * m_local_orientation;

    return deflected_thrust;
}


const btVector3 EngineComponent::getFinalThrustDirection() const{
    const btMatrix3x3& basis = m_owner_part->m_body->getWorldTransform().getBasis();
    return basis * getDeflectedThrustDirection();
}


void EngineComponent::setDeflectionParams(bool yaw, bool pitch, double max_angle_yaw,
                                          double max_angle_pitch){
    if(yaw){
        m_max_angle_yaw = max_angle_yaw;
        m_yaw = true;
    }
    else{
        m_max_angle_yaw = 0.0;
        m_yaw = false;
    }

    if(pitch){
        m_max_angle_pitch = max_angle_pitch;
        m_pitch = true;
    }
    else{
        m_max_angle_pitch = 0.0;
        m_pitch = false;
    }
}


void EngineComponent::getDeflectionParams(bool& yaw, bool& pitch, double& max_angle_yaw, 
                                          double& max_angle_pitch) const{
    yaw = m_yaw;
    pitch = m_pitch;
    max_angle_yaw = m_max_angle_yaw;
    max_angle_pitch = m_max_angle_pitch;
}


double EngineComponent::getMaxAvgThrust() const{
    return m_max_avg_thrust;
}


void EngineComponent::addPropellant(std::uint32_t resource, double flow_rate){
    m_propellants.emplace_back(required_propellant(flow_rate, resource));
}


const std::vector<struct required_propellant>& EngineComponent::getPropellants() const{
    return m_propellants;
}


void EngineComponent::startEngine(){
    if(m_status != ENGINE_STATUS_DAMAGED)
        m_status = ENGINE_STATUS_ON;
}


void EngineComponent::stopEngine(){
    if(m_can_be_stopped)
        m_status = ENGINE_STATUS_OFF;
}


int EngineComponent::getStatus() const{
    return m_status;
}


void EngineComponent::setStop(bool can_be_stopped){
    m_can_be_stopped = can_be_stopped;
}


void EngineComponent::setThrottle(double throttle){
    assert(throttle >= 0.0);
    assert(throttle <= 1.0);
    m_throttle = throttle;
}


double EngineComponent::getThrottle() const{
    return m_throttle;
}


// temp vvv
#define TIME_STEP (1. / 60.)
double EngineComponent::updateResourceFlow(){
    BasePart* requested;
    double min_ratio = 1.0;

    if(m_request_owner)
        requested = m_owner_part;
    else
        requested = m_owner_part->getParent();

    if(!requested)
        return 0.0;

    for(uint i=0; i < m_propellants.size(); i++){
        required_propellant& prop = m_propellants.at(i);

        double ideal_flow_rate = prop.max_flow_rate * m_throttle * TIME_STEP;
        prop.current_flow_rate = ideal_flow_rate;
        // request and update the flow rate
        requested->requestResource(m_owner_part, prop.resource_id, prop.current_flow_rate);
        if(prop.current_flow_rate / ideal_flow_rate < min_ratio)
            min_ratio = prop.current_flow_rate / ideal_flow_rate;
    }
    return min_ratio;
}


void EngineComponent::setOwner(BasePart* owner_part){
    m_owner_part = owner_part;
}


const btVector3 EngineComponent::update(){
    double min_flow = updateResourceFlow();
    btVector3 thrust_direction = getFinalThrustDirection();
    thrust_direction.normalize();
    double thrust = min_flow * m_throttle * m_max_avg_thrust;
    return thrust * thrust_direction.normalize();
}


void EngineComponent::requestResourcesOwner(){
    m_request_owner = true;
}
