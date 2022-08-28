#include <sstream>

#include <imgui.h>

#include "../core/Physics.hpp"
#include "Kinematic.hpp"
#include "Object.hpp"


Kinematic::Kinematic(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID) : 
    Object(model, physics, col_shape, mass, baseID){
}


Kinematic::Kinematic(const Kinematic& object) : Object(object){
}


Kinematic::Kinematic(){

}


Kinematic::~Kinematic(){

}


void Kinematic::update(){
    m_body->clearForces();
    m_body->getMotionState()->setWorldTransform(m_transform);
}


void Kinematic::update(const btTransform& transform){
    m_body->clearForces();
    m_body->getMotionState()->setWorldTransform(transform * m_transform);
}


void Kinematic::update(const btVector3& origin, const btQuaternion& rotation){
    btTransform transform;
    m_body->clearForces();
    transform.setOrigin(origin);
    transform.setRotation(rotation);
    m_body->getMotionState()->setWorldTransform(transform * m_transform);
}


void Kinematic::setTransform(const btTransform& transform){
    m_transform = transform;
}


void Kinematic::setTransform(const btVector3& origin, const btQuaternion& rotation){
    m_transform.setOrigin(origin);
    m_transform.setRotation(rotation);
}


void Kinematic::renderOther(){
}


/*void Kinematic::setTrimesh(std::unique_ptr<iv_array>& array){
    m_trimesh = std::move(array);
}*/
