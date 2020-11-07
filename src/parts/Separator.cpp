#include <sstream>

#include "Separator.hpp"
#include "../AssetManagerInterface.hpp"
#include "../buffers.hpp"


Separator::Separator(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager) : 
    BasePart(model, bt_wrapper, col_shape, mass, baseID, asset_manager){
    m_behaviour = BEHAVIOUR_SEPARATES_SELF;
    m_separate = false;
    m_force = SEPARATOR_MAX_FORCE;
}


Separator::Separator() : BasePart(){
    m_behaviour = BEHAVIOUR_SEPARATES_SELF;
    m_separate = false;
    m_force = SEPARATOR_MAX_FORCE;
}


Separator::Separator(const Separator& engine) : BasePart(engine){
    m_behaviour = BEHAVIOUR_SEPARATES_SELF;
    m_separate = false;
    m_force = engine.m_force;
}


Separator::~Separator(){

}


void Separator::renderOther(){
    if(m_show_editor_menu){
        std::stringstream ss;
        ImVec2 mousepos = ImGui::GetMousePos();
        ss << m_unique_id;

        ImGui::SetNextWindowPos(mousepos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
        ImGui::Begin((m_fancy_name + ss.str()).c_str(), &m_show_editor_menu);

        ImGui::ColorEdit3("Mesh color", m_mesh_color.v);

        ImGui::Separator();

        ImGui::Text("Separator behaviour");
        if(ImGui::RadioButton("Separates itself", m_behaviour == BEHAVIOUR_SEPARATES_SELF)){
            m_behaviour = BEHAVIOUR_SEPARATES_SELF;
        }
        if(ImGui::RadioButton("Separates childs", m_behaviour == BEHAVIOUR_SEPARATES_CHILDS)){
            m_behaviour = BEHAVIOUR_SEPARATES_CHILDS;
        }
        if(ImGui::RadioButton("Separates all", m_behaviour == BEHAVIOUR_SEPARATES_ALL)){
            m_behaviour = BEHAVIOUR_SEPARATES_ALL;
        }

        ImGui::Separator();

        ImGui::Text("Separator force");
        ImGui::SliderFloat("N", &m_force, 0.0f, SEPARATOR_MAX_FORCE);

        ImGui::End();
    }
    else if(m_show_game_menu){
        std::stringstream ss;
        ImVec2 mousepos = ImGui::GetMousePos();
        ss << m_unique_id;

        ImGui::SetNextWindowPos(mousepos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
        ImGui::Begin((m_fancy_name + ss.str()).c_str(), &m_show_game_menu);

        if(ImGui::Button("Separate")){
            m_separate = true;
        }

        ImGui::End();
    }
}


void Separator::update(){
    if(m_separate){
        btVector3 force(0.0, m_force, 0.0);
        m_separate = false;

        if(m_behaviour == BEHAVIOUR_SEPARATES_SELF || m_behaviour == BEHAVIOUR_SEPARATES_ALL){
            if(m_parent){
                struct apply_force_msg msg{m_parent, force, btVector3(0.0, 0.0, 0.0)};
                m_asset_manager->applyForce(msg);

                msg = {this, -1 * force, btVector3(0.0, 0.0, 0.0)};
                m_asset_manager->applyForce(msg);
            }

            decoupleSelf();
        }
        if(m_behaviour == BEHAVIOUR_SEPARATES_CHILDS || m_behaviour == BEHAVIOUR_SEPARATES_ALL){
            for(uint i=0; i < m_childs.size(); i++){
                struct apply_force_msg msg{m_childs.at(i).get(), -1.0 * force, btVector3(0.0, 0.0, 0.0)};
                m_asset_manager->applyForce(msg);

                msg = {this, force, btVector3(0.0, 0.0, 0.0)};
                m_asset_manager->applyForce(msg);
            }

            decoupleChilds();
        }
    }
}


Separator* Separator::clone() const{
    return new Separator(*this);
}


