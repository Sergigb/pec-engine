#include <sstream>

#include <imgui.h>

#include "../core/Physics.hpp"
#include "../core/AssetManagerInterface.hpp"
#include "Kinematic.hpp"
#include "Object.hpp"


Kinematic::Kinematic(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID) : 
    Object(model, physics, col_shape, mass, baseID){
    m_velocity = btVector3(0.0, 0.0, 0.0);
}


Kinematic::Kinematic(const Kinematic& object) : Object(object){
    m_velocity = btVector3(0.0, 0.0, 0.0);
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
    std::stringstream ss;
    ss << m_unique_id;

    ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
    ImGui::Begin((m_fancy_name + ss.str()).c_str());

    float v[3];
    v[0] = m_velocity.getX();
    v[1] = m_velocity.getY();
    v[2] = m_velocity.getZ();

    ImGui::SliderFloat3("Velocity", v, -10.0f, 10.0f);

    m_velocity = btVector3(v[0], v[1], v[2]);

    ImGui::End();
}


void Kinematic::setTrimesh(std::unique_ptr<iv_array>& array){
    m_trimesh = std::move(array);
}


