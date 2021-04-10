#include <sstream>
#include <functional>
#include <iostream>

#include "GenericEngine.hpp"
#include "../Resource.hpp"
#include "../Vessel.hpp"
#include "../../core/maths_funcs.hpp"
#include "../../core/log.hpp"
#include "../../core/AssetManagerInterface.hpp"

GenericEngine::GenericEngine(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager) : 
    BasePart(model, physics, col_shape, mass, baseID, asset_manager){
    std::hash<std::string> str_hash;

    m_engine_status = false;
    m_thrust = 1.0f;
    m_liquid_hydrogen_id = str_hash("liquid_hydrogen");
    m_liquid_oxygen_id = str_hash("liquid_oxygen");
}


GenericEngine::GenericEngine() : BasePart() {
    std::hash<std::string> str_hash;

    m_engine_status = false;
    m_thrust = 1.0f;
    m_liquid_hydrogen_id = str_hash("liquid_hydrogen");
    m_liquid_oxygen_id = str_hash("liquid_oxygen");
}


GenericEngine::GenericEngine(const GenericEngine& engine) : BasePart(engine) {
    m_engine_status = false;
    m_thrust = 1.0f;
    m_liquid_hydrogen_id = engine.m_liquid_hydrogen_id;
    m_liquid_oxygen_id = engine.m_liquid_oxygen_id;
}


GenericEngine::~GenericEngine(){

}


void GenericEngine::renderOther(){
    if(m_show_editor_menu){
        std::stringstream ss;
        ImVec2 mousepos = ImGui::GetMousePos();
        ss << m_unique_id;

        ImGui::SetNextWindowPos(mousepos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
        ImGui::Begin((m_fancy_name + ss.str()).c_str(), &m_show_editor_menu);

        ImGui::ColorEdit3("Mesh color", m_mesh_color.v);

        for(uint i=0; i < m_resources.size(); i++){
            std::string rname;
            m_resources.at(i).resource->getFancyName(rname);
            ImGui::SliderFloat(rname.c_str(), &m_resources.at(i).mass, 0.0f, m_resources.at(i).max_mass);
        }

        ImGui::SliderFloat("Initial thrust", &m_thrust, 0.0f, 1.0f);

        ImGui::End();
    }
    else if(m_show_game_menu){
        std::stringstream ss;
        ImVec2 mousepos = ImGui::GetMousePos();
        ss << m_unique_id;
        std::string engine_status_button = m_engine_status ? "Stop engine" : "Start engine";

        ImGui::SetNextWindowPos(mousepos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
        ImGui::Begin((m_fancy_name + ss.str()).c_str(), &m_show_game_menu);

        if(ImGui::Button(engine_status_button.c_str())){
            m_engine_status = !m_engine_status;
        }

        ImGui::SliderFloat("Thrust", &m_thrust, 0.0f, 1.0f);

        ImGui::End();
    }
}


void GenericEngine::update(){
    if(m_engine_status && m_parent && m_thrust){
        float liq_hyd = 5.0f * m_thrust, liq_oxy = 2.0f * m_thrust;
        float flow_hyd = liq_hyd, flow_oxy = liq_oxy;
        float flow = 1.0f; // the quantities are made up
        float max_gimbal_angle = 10.5f * ONE_DEG_IN_RAD; // all this stuff won't be hardcoded in the future
        btMatrix3x3& basis = m_body->getWorldTransform().getBasis();
        btMatrix3x3 gimbal;
        btVector3 force;

        m_parent->requestResource(this, m_liquid_hydrogen_id, flow_hyd);
        m_parent->requestResource(this, m_liquid_oxygen_id, flow_oxy);

        flow = flow_hyd / liq_hyd > flow_oxy / liq_oxy ? flow_oxy / liq_oxy : flow_hyd / liq_hyd; // this will be re-thinked
        force = btVector3(0.0, flow * 2500.0 * m_thrust, 0.0);

        // I'm using Y as the roll, this is wrong. In the future I'll make sure Z is the forward vector, which will make it the roll vector
        gimbal.setEulerZYX(m_vessel->getYaw() * max_gimbal_angle, 0.0, m_vessel->getPitch() * max_gimbal_angle);
        force = basis * gimbal * force;
        m_asset_manager->applyForce(this, force, btVector3(0.0, 0.0, 0.0));
    }
}

GenericEngine* GenericEngine::clone() const{
    return new GenericEngine(*this);
}


void GenericEngine::action(int action){
    if(action == PART_ACTION_ENGINE_START){
        m_engine_status = true;
    }
    else if(action == PART_ACTION_ENGINE_TOGGLE){
        m_engine_status = !m_engine_status;
    }
    else{
        std::cerr << "GenericEngine::action: got an invalid action value: " << action << std::endl;
        log("GenericEngine::action: got an invalid action value: ", action);
    }
}

