#include <sstream>

#include <imgui.h>

#include "../core/Physics.hpp"
#include "../core/AssetManagerInterface.hpp"
#include "Kinematic.hpp"
#include "Object.hpp"


Kinematic::Kinematic(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID) : 
    Object(model, physics, col_shape, mass, baseID), m_velocity(btVector3(0.0, 0.0, 0.0)){
}


Kinematic::Kinematic(const Kinematic& object) : Object(object), m_velocity(btVector3(0.0, 0.0, 0.0)){
}


Kinematic::Kinematic(){

}


Kinematic::~Kinematic(){

}


void Kinematic::update(){
    btTransform transform;
    m_body->clearForces();
    m_body->getMotionState()->getWorldTransform(transform);
    transform.getOrigin() += m_velocity;
    m_body->getMotionState()->setWorldTransform(transform);
}


void Kinematic::renderOther(){
}


void Kinematic::setTrimesh(std::unique_ptr<iv_array>& array){
    m_trimesh = std::move(array);
}


