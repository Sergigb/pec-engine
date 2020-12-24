#include <sstream>

#include <imgui.h>

#include "Kinematic.hpp"
#include "Object.hpp"
#include "AssetManagerInterface.hpp"


Kinematic::Kinematic(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID) : 
    Object(model, bt_wrapper, col_shape, mass, baseID){

}


Kinematic::Kinematic(const Kinematic& object) : Object(object){

}


Kinematic::Kinematic(){

}


Kinematic::~Kinematic(){

}


void Kinematic::update(){
    
}


void Kinematic::renderOther(){
    std::stringstream ss;
    ss << m_unique_id;

    //ImGui::SetNextWindowPos(mousepos, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
    ImGui::Begin((m_fancy_name + ss.str()).c_str());

    ImGui::Text("Test");

    ImGui::End();
}


void Kinematic::setTrimesh(std::unique_ptr<btTriangleIndexVertexArray>& array){
    m_trimesh = std::move(array);
}


