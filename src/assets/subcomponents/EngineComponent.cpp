#include <cassert>

#include "EngineComponent.hpp"
#include "../Vessel.hpp"
#include "../BasePart.hpp"


EngineComponent::EngineComponent(BasePart* parent_part, const btVector3& origin,
                                 const btVector3& orientation) : m_local_origin(origin),
                                 m_local_orientation(orientation) {
    assert(parent_part);

    m_local_orientation = m_local_orientation.normalize();

    m_max_angle_yaw = 0.0;
    m_max_angle_pitch = 0.0;
    m_yaw = false;
    m_pitch = false;

    m_parent_part = parent_part;

}


EngineComponent::EngineComponent(){
    m_local_origin = btVector3(0.0, 0.0, 0.0);
    m_parent_part = nullptr;
    m_local_orientation = btVector3(0., -1., 0.);

    m_max_angle_yaw = 0.0;
    m_max_angle_pitch = 0.0;
    m_yaw = false;
    m_pitch = false;
}


const btVector3& EngineComponent::getThrustOrigin() const{
    return m_local_origin;
}


const btVector3& EngineComponent::getThrustOrientation() const{
    return m_local_orientation;
}


const btMatrix3x3 EngineComponent::getThrustDeflectionBasis() const{
    btMatrix3x3 deflect_basis;
    const Vessel* vessel = m_parent_part->getVessel();

    deflect_basis.setEulerZYX(vessel->getYaw() * m_max_angle_yaw, 0.0,
                              vessel->getPitch() * m_max_angle_pitch);

    return deflect_basis;
}


const btVector3 EngineComponent::getDeflectedThrust() const{
    const btMatrix3x3& deflect_basis = getThrustDeflectionBasis();
    const btVector3 deflected_thrust = deflect_basis * m_local_orientation;

    return deflected_thrust;
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


