#include "Separator.hpp"


Separator::Separator(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager) : 
    BasePart(model, bt_wrapper, col_shape, mass, baseID, asset_manager){
    m_behaviour = BEHAVIOUR_SEPARATES_SELF;
    m_separate = false;
}


Separator::Separator() : BasePart(){
    m_behaviour = BEHAVIOUR_SEPARATES_SELF;
    m_separate = false;
}


Separator::Separator(const Separator& engine) : BasePart(engine){
    m_behaviour = BEHAVIOUR_SEPARATES_SELF;
    m_separate = false;
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
        m_separate = false;
        if(m_behaviour == BEHAVIOUR_SEPARATES_CHILDS){
            decoupleChilds();
        }
        else if(m_behaviour == BEHAVIOUR_SEPARATES_SELF){
            decoupleSelf();
        }
        else if(m_behaviour == BEHAVIOUR_SEPARATES_ALL){
            decoupleSelf();
            decoupleChilds();
        }
    }
}


Separator* Separator::clone() const{
    return new Separator(*this);
}


