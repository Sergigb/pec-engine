#include <sstream>
#include <functional>
#include <iostream>

#include "P80.hpp"
#include "../Resource.hpp"
#include "../AssetManagerInterface.hpp"
#include "../Vessel.hpp"
#include "../maths_funcs.hpp"
#include "../log.hpp"


#define EARTH_GRAVITY 9.81

P80::P80(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager) : 
    BasePart(model, physics, col_shape, mass, baseID, asset_manager){
    std::hash<std::string> str_hash;

    m_engine_status = ENGINE_OFF;
    m_htpb_id = str_hash("htpb");
}


P80::P80() : BasePart() {
    std::hash<std::string> str_hash;

    m_engine_status = ENGINE_OFF;
    m_htpb_id = str_hash("htpb");
}


P80::P80(const P80& engine) : BasePart(engine) {
    m_engine_status = ENGINE_OFF;
    m_htpb_id = engine.m_htpb_id;
}


P80::~P80(){

}


void P80::renderOther(){
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

        ss.str("");
        ss.clear();
        ss << "Vacuum thrust: " << AVERAGE_THRUST / 1000.0 << " kN";
        ImGui::Text(ss.str().c_str());

        ss.str("");
        ss.clear();
        ss << "Specific impulse: " << AVERAGE_THRUST / MASS_FLOW_RATE / EARTH_GRAVITY << " s";
        ImGui::Text(ss.str().c_str());

        ImGui::End();
    }
    else if(m_show_game_menu){
        std::stringstream ss;
        ImVec2 mousepos = ImGui::GetMousePos();
        ss << m_unique_id;

        ImGui::SetNextWindowPos(mousepos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
        ImGui::Begin((m_fancy_name + ss.str()).c_str(), &m_show_game_menu);

        switch(m_engine_status){
            case(ENGINE_OFF):
            ImGui::Text("Engine status: off");
            if(ImGui::Button("Start engine"))
                m_engine_status = ENGINE_ON;
            break;

            case(ENGINE_ON):
            ImGui::Text("Engine status: on");
            ImGui::Text("Solid-fuel engines cannot be turned off");
            break;

            case(ENGINE_DEPLETED):
            ImGui::Text("Engine status: off (fuel depleted)");
            break;
        }

        ImGui::Separator();
        for(uint i=0; i < m_resources.size(); i++){
            std::string rname;
            m_resources.at(i).resource->getFancyName(rname);
            ImGui::Text(rname.c_str());
            ImGui::ProgressBar(m_resources.at(i).mass / m_resources.at(i).max_mass);
        }

        ImGui::End();
    }
}


// temp vvv
#define TIME_STEP (1. / 60.)

void P80::update(){
    double temp_mass = m_dry_mass;
    for(uint i=0; i < m_resources.size(); i++){
        temp_mass += m_resources.at(i).mass;
    }

    if(temp_mass != m_mass){
        m_mass = temp_mass;
        m_asset_manager->setMassProps(this, m_mass);
    }

    if(m_engine_status == ENGINE_ON){
        float current_flow = MASS_FLOW_RATE * TIME_STEP;

        btMatrix3x3& basis = m_body->getWorldTransform().getBasis();
        btMatrix3x3 gimbal;
        btVector3 force;

        // request to ourselves
        requestResource(this, m_htpb_id, current_flow);

        if(current_flow == 0.0f){
            m_engine_status = ENGINE_DEPLETED;
            return;
        }

        // thrust = flow rate ratio (output / max) * avg_thrust
        force = btVector3(0.0, (current_flow / (MASS_FLOW_RATE * TIME_STEP)) * AVERAGE_THRUST, 0.0);

        gimbal.setEulerZYX(m_vessel->getYaw() * MAX_DEFLECTION_ANGLE, 0.0, m_vessel->getPitch() * MAX_DEFLECTION_ANGLE);
        force = basis * gimbal * force;
        m_asset_manager->applyForce(this, force, basis * btVector3(0.0, -5.0, 0.0));
    }
}

P80* P80::clone() const{
    return new P80(*this);
}


void P80::action(int action){
    if(action == PART_ACTION_ENGINE_START){
        m_engine_status = ENGINE_ON;
    }
    else{
        std::cerr << "P80::action: got an invalid action value: " << action << std::endl;
        log("P80::action: got an invalid action value: ", action);
    }
}

