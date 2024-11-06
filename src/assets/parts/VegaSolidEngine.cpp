#include <sstream>
#include <functional>
#include <iostream>

#include <tinyxml2.h>

#include "VegaSolidEngine.hpp"
#include "../Resource.hpp"
#include "../Model.hpp"
#include "../Vessel.hpp"
#include "../../core/AssetManagerInterface.hpp"
#include "../../core/maths_funcs.hpp"
#include "../../core/log.hpp"
#include "../../core/RenderContext.hpp"
#include "../../core/loading/xml_utils.hpp"



#define EARTH_GRAVITY 9.81


VegaSolidEngine::VegaSolidEngine(Model* model, Physics* physics, btCollisionShape* col_shape,
                                 btScalar mass, int baseID,
                                 AssetManagerInterface* asset_manager) : 
    BasePart(model, physics, col_shape, mass, baseID, asset_manager){
    init();
}


VegaSolidEngine::VegaSolidEngine() : BasePart() {

    init();
}


void VegaSolidEngine::init(){   
    m_fairing_model = nullptr;

    m_separate = false;
    m_separation_force = 100000.0;
}


VegaSolidEngine::VegaSolidEngine(const VegaSolidEngine& engine) : BasePart(engine) {
    m_fairing_model = engine.m_fairing_model;
    m_separate = false;
    m_separation_force = engine.m_separation_force;
    m_main_engine = engine.m_main_engine;
    m_main_engine->setOwner(this);
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
        ImGui::Begin((m_fancy_name + "##" + ss.str()).c_str(), &m_show_editor_menu);

        ImGui::ColorEdit3("Mesh color", m_mesh_color.v);

        for(uint i=0; i < m_resources.size(); i++){
            const std::string& rname = m_resources.at(i).resource->getFancyName();
            float current_mass = m_resources.at(i).mass;
            if(ImGui::SliderFloat(rname.c_str(), &current_mass,
                               0.0f, m_resources.at(i).max_mass))
                m_resources.at(i).mass = current_mass;
        }

        ss.str("");
        ss.clear();
        ss << "Vacuum thrust: N/A";// << m_average_thrust / 1000.0 << " kN";
        ImGui::Text(ss.str().c_str());

        ss.str("");
        ss.clear();
        ss << "Specific impulse: N/A";// " << m_average_thrust / m_mass_flow_rate / EARTH_GRAVITY << " s";
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

        switch(m_main_engine->getStatus()){
            case(ENGINE_STATUS_ON):
            ImGui::Text("Engine status: off");
            if(ImGui::Button("Start engine"))
                action(PART_ACTION_ENGINE_START);
            break;

            case(ENGINE_STATUS_OFF):
            ImGui::Text("Engine status: on");
            ImGui::Text("Solid-fuel engines cannot be turned off");
            break;

            case(ENGINE_STATUS_DAMAGED):
            ImGui::Text("Engine status: damaged");
            break;
        }

        ImGui::Separator();
        for(uint i=0; i < m_resources.size(); i++){
            const std::string& rname = m_resources.at(i).resource->getFancyName();
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

    if(m_main_engine->getStatus() == ENGINE_STATUS_ON){
        const btVector3 force = m_main_engine->update();
        m_asset_manager->applyForce(this, force, btVector3(0.0, 0.0, 0.0));
    }

    if(m_separate){
        btTransform transform;
        m_body->getMotionState()->getWorldTransform(transform);
        const btMatrix3x3& part_rotation = transform.getBasis();
        btVector3 force = part_rotation * btVector3(0.0, m_separation_force, 0.0);  // todo: find the real force

        m_separate = false;

        m_asset_manager->applyForce(m_parent, force, btVector3(0.0, 0.0, 0.0));
        m_asset_manager->applyForce(this, -1 * force, btVector3(0.0, 0.0, 0.0));

        decoupleSelf();
    }
}

VegaSolidEngine* VegaSolidEngine::clone() const{
    return new VegaSolidEngine(*this);
}


void VegaSolidEngine::action(int action){
    switch(action){
        case PART_ACTION_ENGINE_START:
        case PART_ACTION_ENGINE_TOGGLE:
            if(m_main_engine->getStatus() == ENGINE_STATUS_OFF)
                m_main_engine->startEngine();
            else if(m_main_engine->getStatus() == ENGINE_STATUS_ON){
                std::cerr << "VegaSolidEngine::action: can't stop solid engines: "
                          << action << std::endl;
                log("VegaSolidEngine::action: can't stop solid engines: ", action);
            }
            break;
        case PART_ACTION_SEPARATE:
            m_separate = true;
            break;
        default:
            std::cerr << "VegaSolidEngine::action: got an invalid action value: "
                      << action << std::endl;
            log("VegaSolidEngine::action: got an invalid action value: ", action);
    }
}


int VegaSolidEngine::render(){
    math::mat4 body_transform = getRigidBodyTransformSingle();
    if(m_has_transform){
        body_transform = body_transform * m_mesh_transform;
    }

    if(m_vessel){
        m_model->setMeshColor(math::vec4(m_mesh_color, m_alpha));
    }
    else{
        m_model->setMeshColor(math::vec4(m_mesh_color, 0.5));
    }

    if(m_childs.size() && m_fairing_model){
        if(m_vessel){
            m_fairing_model->setMeshColor(math::vec4(m_mesh_color, m_alpha));
        }
        else{
            m_fairing_model->setMeshColor(math::vec4(m_mesh_color, 0.5));
        }        
        m_fairing_model->render(body_transform);
    }

    return m_model->render(body_transform);
}


int VegaSolidEngine::render(const math::mat4& body_transform){
    if(m_vessel){
        m_model->setMeshColor(math::vec4(m_mesh_color, m_alpha));
    }
    else{
        m_model->setMeshColor(math::vec4(m_mesh_color, 0.5));
    }

    if(m_childs.size() && m_fairing_model){
        if(m_vessel){
            m_fairing_model->setMeshColor(math::vec4(m_mesh_color, m_alpha));
        }
        else{
            m_fairing_model->setMeshColor(math::vec4(m_mesh_color, 0.5));
        }        
        m_fairing_model->render(body_transform);
    }

    return m_model->render(m_has_transform ? body_transform * m_mesh_transform : body_transform);
}


typedef tinyxml2::XMLElement xmle;
int VegaSolidEngine::loadCustom(const tinyxml2::XMLElement* elem){
    const xmle* stats_elem = get_element(elem, "engine_stats");
    const xmle* eject_elem = get_element(elem, "separation_force");
    const xmle* fairing_elem = get_element(elem, "fairing_model_path", true);
    const char* fairing_model_path;
    double average_thrust, max_deflection_angle;

    if(!eject_elem){
        std::cerr << "VegaSolidEngine::loadCustom: Missing ejection force element in engine "
                     "defined in " << elem->GetLineNum() << std::endl;
        log("VegaSolidEngine::loadCustom: Missing ejection force element in engine defined in ",
            elem->GetLineNum());

        return EXIT_FAILURE;
    }

    if(eject_elem->QueryDoubleText(&m_separation_force))
        return EXIT_FAILURE;

    // load fairing
    if(fairing_elem){
        fairing_model_path = fairing_elem->GetText();

        std::unique_ptr<Model> fairing(new Model(fairing_model_path, nullptr,
                                                 SHADER_PHONG_BLINN_NO_TEXTURE, 
                                                 math::vec3(0.75, 0.75, 0.75)));
        m_fairing_model = fairing.get();
        m_asset_manager->storeModel(std::move(fairing));
    }
    else
        m_fairing_model = nullptr;

    if(!stats_elem){
        std::cerr << "VegaSolidEngine::loadCustom: Missing stats elements in engine defined in "
                  << elem->GetLineNum() << std::endl;
        log("VegaSolidEngine::loadCustom: Missing stats elements in engine defined in ",
            elem->GetLineNum());

        return EXIT_FAILURE;
    }

    if(get_double(stats_elem, "avg_thrust", average_thrust) == EXIT_FAILURE)
        return EXIT_FAILURE;
    if(get_double(stats_elem, "max_deflect_angle", max_deflection_angle) == EXIT_FAILURE)
        return EXIT_FAILURE;

    const xmle* origin_elem = get_element(stats_elem, "origin");
    const xmle* orientation_elem = get_element(stats_elem, "orientation");

    if(!origin_elem){
        std::cerr << "VegaSolidEngine::loadCustom: Missing origin for engine defined in "
                  << stats_elem->GetLineNum() << std::endl;
        log("VegaSolidEngine::loadCustom: Missing origin for engine defined in ",
            stats_elem->GetLineNum());
        return EXIT_FAILURE;
    }

    if(!orientation_elem){
        std::cerr << "VegaSolidEngine::loadCustom: Missing orientation for engine defined in "
                  << stats_elem->GetLineNum() << std::endl;
        log("VegaSolidEngine::loadCustom: Missing orientation for engine defined in ",
            stats_elem->GetLineNum());

        return EXIT_FAILURE;
    }
    double x, y, z, ox, oy, oz;

    if(get_double(origin_elem, "x", x))
        return EXIT_FAILURE;
    if(get_double(origin_elem, "y", y))
        return EXIT_FAILURE;
    if(get_double(origin_elem, "z", z))
        return EXIT_FAILURE;

    if(get_double(orientation_elem, "x", ox))
        return EXIT_FAILURE;
    if(get_double(orientation_elem, "y", oy))
        return EXIT_FAILURE;
    if(get_double(orientation_elem, "z", oz))
        return EXIT_FAILURE;

    m_engine_list.emplace_back(new EngineComponent(this, btVector3(x, y, z),
                               btVector3(ox, oy, oz), average_thrust));

    m_main_engine = m_engine_list.at(0);

    const xmle* resources_elem = get_element(stats_elem, "required_resource");

     if(!resources_elem){
        std::cerr << "VegaSolidEngine::loadCustom: Missing resources element in engine "
                     "stats defined in " << stats_elem->GetLineNum() << std::endl;
        log("VegaSolidEngine::loadCustom: Missing resources element in engine stats defined in ",
            stats_elem->GetLineNum());

        return EXIT_FAILURE;
    }

    const xmle* resource_elem = resources_elem->FirstChildElement("resource");
    std::hash<std::string> str_hash;

    while(resource_elem){
        double flow_rate;
        const char* resource_name;

        if(get_double(resource_elem, "mass_flow_rate", flow_rate) == EXIT_FAILURE)
            return EXIT_FAILURE;

        if(get_string(resource_elem, "resource_name", &resource_name) == EXIT_FAILURE)
            return EXIT_FAILURE;

        m_main_engine->addPropellant(str_hash(resource_name), flow_rate);

        resource_elem = resource_elem->NextSiblingElement("resource");
    }

    m_main_engine->setDeflectionParams(true, true, max_deflection_angle, max_deflection_angle);
    m_main_engine->setThrottle(1.0);
    m_main_engine->requestResourcesOwner();

    return EXIT_SUCCESS;
}
