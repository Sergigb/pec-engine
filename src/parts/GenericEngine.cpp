#include "GenericEngine.hpp"
#include "../Resource.hpp"
#include "../AssetManagerInterface.hpp"


GenericEngine::GenericEngine(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager) : 
    BasePart(model, bt_wrapper, col_shape, mass, baseID, asset_manager){

    m_engine_status = false;
}


GenericEngine::GenericEngine() : BasePart() {
    m_engine_status = false;
}


GenericEngine::GenericEngine(const GenericEngine& engine) : BasePart(engine) {
    m_engine_status = false;
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

        ImGui::End();
    }
    else if(m_show_game_menu){
        std::stringstream ss;
        ImVec2 mousepos = ImGui::GetMousePos();
        ss << m_unique_id;

        ImGui::SetNextWindowPos(mousepos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
        ImGui::Begin((m_fancy_name + ss.str()).c_str(), &m_show_game_menu);

        if(ImGui::Button("Start engine")){
            m_engine_status = !m_engine_status;
        }

        ImGui::End();
    }
}


void GenericEngine::update(){
    if(m_engine_status){
        btVector3 force(0.0, 2500.0, 0.0);
        btMatrix3x3& basis = m_body->getWorldTransform().getBasis();

        force = force * basis.inverse();
        struct apply_force_msg msg{this, force, btVector3(0.0, 0.0, 0.0)};
        m_asset_manager->applyForce(msg);
    }
}


GenericEngine* GenericEngine::clone() const{
    return new GenericEngine(*this);
}

