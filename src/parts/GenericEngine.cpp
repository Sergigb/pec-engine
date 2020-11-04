#include <sstream>
#include <functional>

#include "GenericEngine.hpp"
#include "../Resource.hpp"
#include "../AssetManagerInterface.hpp"
#include "../buffers.hpp"


GenericEngine::GenericEngine(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager) : 
    BasePart(model, bt_wrapper, col_shape, mass, baseID, asset_manager){
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
    if(m_engine_status){
        if(m_parent){
            float liq_hyd = 50.0f, liq_oxy = 10.0f;
            float flow = 1.0f; // the quantities are made up
            btMatrix3x3& basis = m_body->getWorldTransform().getBasis();
            btVector3 force;

            m_parent->requestResource(this, m_liquid_hydrogen_id, liq_hyd);
            m_parent->requestResource(this, m_liquid_oxygen_id, liq_oxy);

            flow = liq_hyd / 10.0f > liq_oxy / 4.0 ? liq_oxy / 4.0 : liq_hyd / 10.0f; // not sure about this

            force = btVector3(0.0, flow * m_thrust * 2500.0, 0.0);

            force = basis * force;
            struct apply_force_msg msg{this, force, btVector3(0.0, 0.0, 0.0)};
            m_asset_manager->applyForce(msg);
        }
    }
}


GenericEngine* GenericEngine::clone() const{
    return new GenericEngine(*this);
}

