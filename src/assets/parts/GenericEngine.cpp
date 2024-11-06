#include <sstream>
#include <functional>
#include <iostream>

#include "GenericEngine.hpp"
#include "../Resource.hpp"
#include "../Vessel.hpp"
#include "../../core/maths_funcs.hpp"
#include "../../core/log.hpp"
#include "../../core/AssetManagerInterface.hpp"

GenericEngine::GenericEngine(Model* model, Physics* physics, btCollisionShape* col_shape,
                             btScalar mass, int baseID, AssetManagerInterface* asset_manager) : 
    BasePart(model, physics, col_shape, mass, baseID, asset_manager){
    std::hash<std::string> str_hash;

    m_engine_list.emplace_back(new EngineComponent(this, btVector3(0., 0.064, 0.),
                               btVector3(0., 1., 0.), 1000000.));

    m_main_engine = m_engine_list.at(0);
    m_main_engine->addPropellant(str_hash("liquid_hydrogen"), 300.);
    m_main_engine->addPropellant(str_hash("liquid_oxygen"), 60.);
    m_main_engine->setDeflectionParams(true, true, 10., 10.);
    m_main_engine->setThrottle(1.0);
}


GenericEngine::GenericEngine() : BasePart() {

}


GenericEngine::GenericEngine(const GenericEngine& engine) : BasePart(engine) {
    m_main_engine = engine.m_main_engine;
    m_main_engine->setOwner(this);
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
        ImGui::Begin((m_fancy_name + "##"+ ss.str()).c_str(), &m_show_editor_menu);

        ImGui::ColorEdit3("Mesh color", m_mesh_color.v);

        for(uint i=0; i < m_resources.size(); i++){
            const std::string& rname = m_resources.at(i).resource->getFancyName();
            float current_mass = m_resources.at(i).mass;
            ImGui::SliderFloat(rname.c_str(), &current_mass, 0.0f,
                               (float)m_resources.at(i).max_mass);
            m_resources.at(i).mass = current_mass;
        }

        float throttle = m_main_engine->getThrottle();
        if(ImGui::SliderFloat("Initial throttle", &throttle, 0.0f, 1.0f))
            m_main_engine->setThrottle((float)throttle);

        ImGui::End();
    }
    else if(m_show_game_menu){
        std::stringstream ss;
        ImVec2 mousepos = ImGui::GetMousePos();
        int engine_status = m_main_engine->getStatus();

        ss << m_unique_id;
        std::string engine_status_button = engine_status == ENGINE_STATUS_ON
                                           ? "Stop engine" : "Start engine";

        ImGui::SetNextWindowPos(mousepos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
        ImGui::Begin((m_fancy_name + ss.str()).c_str(), &m_show_game_menu);

        if(ImGui::Button(engine_status_button.c_str())){
            if(engine_status == ENGINE_STATUS_ON)
                m_main_engine->stopEngine();
            else
                m_main_engine->startEngine();
        }

        float throttle = m_main_engine->getThrottle();
        if(ImGui::SliderFloat("Throttle", &throttle, 0.0f, 1.0f))
            m_main_engine->setThrottle((float)throttle);

        ImGui::End();
    }
}


void GenericEngine::update(){
    if(m_main_engine->getStatus() == ENGINE_STATUS_ON && m_parent){
        const btVector3 force = m_main_engine->update();
        m_asset_manager->applyForce(this, force, btVector3(0.0, 0.0, 0.0));
    }
}

GenericEngine* GenericEngine::clone() const{
    return new GenericEngine(*this);
}


void GenericEngine::action(int action){
    if(action == PART_ACTION_ENGINE_START){
        m_main_engine->startEngine();
    }
    else if(action == PART_ACTION_ENGINE_TOGGLE){
        int status = m_main_engine->getStatus();
        if(status == ENGINE_STATUS_ON)
            m_main_engine->startEngine();
        else if(status == ENGINE_STATUS_OFF)
            m_main_engine->stopEngine();
    }
    else{
        std::cerr << "GenericEngine::action: got an invalid action value: " << action << std::endl;
        log("GenericEngine::action: got an invalid action value: ", action);
    }
}

