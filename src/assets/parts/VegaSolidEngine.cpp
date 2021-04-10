#include <sstream>
#include <functional>
#include <iostream>

#include "VegaSolidEngine.hpp"
#include "../Resource.hpp"
#include "../Model.hpp"
#include "../Vessel.hpp"
#include "../../core/AssetManagerInterface.hpp"
#include "../../core/maths_funcs.hpp"
#include "../../core/log.hpp"



#define EARTH_GRAVITY 9.81


VegaSolidEngine::VegaSolidEngine(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager) : 
    BasePart(model, physics, col_shape, mass, baseID, asset_manager){
    init();
}


VegaSolidEngine::VegaSolidEngine() : BasePart() {

    init();
}


void VegaSolidEngine::init(){
    std::hash<std::string> str_hash;

    m_engine_status = ENGINE_OFF;
    m_htpb_id = str_hash("htpb");
    m_fairing_model = nullptr;

    m_average_thrust = 0.0;
    m_mass_flow_rate = 0.0;
    m_max_deflection_angle = 0.0;
}


VegaSolidEngine::VegaSolidEngine(const VegaSolidEngine& engine) : BasePart(engine) {
    m_engine_status = ENGINE_OFF;
    m_htpb_id = engine.m_htpb_id;
    m_fairing_model = engine.m_fairing_model;
    m_average_thrust = engine.m_average_thrust;
    m_mass_flow_rate = engine.m_mass_flow_rate;
    m_max_deflection_angle = engine.m_max_deflection_angle;
}


VegaSolidEngine::~VegaSolidEngine(){

}


void VegaSolidEngine::renderOther(){
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
        ss << "Vacuum thrust: " << m_average_thrust / 1000.0 << " kN";
        ImGui::Text(ss.str().c_str());

        ss.str("");
        ss.clear();
        ss << "Specific impulse: " << m_average_thrust / m_mass_flow_rate / EARTH_GRAVITY << " s";
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

void VegaSolidEngine::update(){
    double temp_mass = m_dry_mass;
    for(uint i=0; i < m_resources.size(); i++){
        temp_mass += m_resources.at(i).mass;
    }

    if(temp_mass != m_mass){
        m_mass = temp_mass;
        m_asset_manager->setMassProps(this, m_mass);
    }

    if(m_engine_status == ENGINE_ON){
        float current_flow = m_mass_flow_rate * TIME_STEP;

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
        force = btVector3(0.0, (current_flow / (m_mass_flow_rate * TIME_STEP)) * m_average_thrust, 0.0);

        gimbal.setEulerZYX(m_vessel->getYaw() * m_max_deflection_angle, 0.0, m_vessel->getPitch() * m_max_deflection_angle);
        force = basis * gimbal * force;
        m_asset_manager->applyForce(this, force, basis * btVector3(0.0, -5.0, 0.0));
    }
}

VegaSolidEngine* VegaSolidEngine::clone() const{
    return new VegaSolidEngine(*this);
}


void VegaSolidEngine::action(int action){
    if(action == PART_ACTION_ENGINE_START){
        m_engine_status = ENGINE_ON;
    }
    else{
        std::cerr << "VegaSolidEngine::action: got an invalid action value: " << action << std::endl;
        log("VegaSolidEngine::action: got an invalid action value: ", action);
    }
}


int VegaSolidEngine::render(){
    math::mat4 body_transform = getRigidBodyTransformSingle();
 
    if(m_has_transform){
        body_transform = body_transform * m_mesh_transform;
    }

    if(m_vessel){
        m_model->setMeshColor(math::vec4(m_mesh_color, 1.0));
    }
    else{
        m_model->setMeshColor(math::vec4(m_mesh_color, 0.5));
    }

    if(m_childs.size() && m_fairing_model){
        if(m_vessel){
            m_fairing_model->setMeshColor(math::vec4(m_mesh_color, 1.0));
        }
        else{
            m_fairing_model->setMeshColor(math::vec4(m_mesh_color, 0.5));
        }        
        m_fairing_model->render(body_transform);
    }

    return m_model->render(body_transform);
}


int VegaSolidEngine::render(math::mat4 body_transform){
    if(m_has_transform){
        body_transform = body_transform * m_mesh_transform;
    }

    if(m_vessel){
        m_model->setMeshColor(math::vec4(m_mesh_color, 1.0));
    }
    else{
        m_model->setMeshColor(math::vec4(m_mesh_color, 0.5));
    }

    if(m_childs.size() && m_fairing_model){
        if(m_vessel){
            m_fairing_model->setMeshColor(math::vec4(m_mesh_color, 1.0));
        }
        else{
            m_fairing_model->setMeshColor(math::vec4(m_mesh_color, 0.5));
        }        
        m_fairing_model->render(body_transform);
    }

    return m_model->render(body_transform);
}


void VegaSolidEngine::setFairingModel(Model* fairing_model){
    m_fairing_model = fairing_model;
}


void VegaSolidEngine::setEngineStats(double average_thrust, double mass_flow_rate, double max_deflection_angle){
    m_average_thrust = average_thrust;
    m_mass_flow_rate = mass_flow_rate;
    m_max_deflection_angle = max_deflection_angle;
}

