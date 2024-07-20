#include <iomanip>
#include <algorithm>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "PlanetariumGUI.hpp"
#include "../Text2D.hpp"
#include "../../core/RenderContext.hpp"
#include "../../core/log.hpp"
#include "../../core/Camera.hpp"
#include "../../core/Physics.hpp"
#include "../../core/AssetManager.hpp"
#include "../../core/BaseApp.hpp"
#include "../../core/WindowHandler.hpp"
#include "../../assets/PlanetarySystem.hpp"
#include "../../assets/Vessel.hpp"
#include "../../game_components/GamePlanetarium.hpp"


const float c[4] = {.85f, .85f, .85f, 1.f};


PlanetariumGUI::PlanetariumGUI(const FontAtlas* atlas, const BaseApp* app){
    m_font_atlas = atlas;
    m_app = app;
    m_render_context = app->getRenderContext();
    m_physics = app->getPhysics();;
    m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
    m_camera = app->getCamera();
    m_fb_update = true;
    m_delta_t = 1 / 60.;
    m_selected_planet = 0;
    m_asset_manager = app->getAssetManager();
    m_freecam = false;
    m_target_fade = 0.0;
    m_show_settings = false;
    m_predictor_steps = 400;
    m_predictor_period = 365.f;

    buildSystemGUIData();

    m_main_text.reset(new Text2D(m_fb_width, m_fb_height,
                      m_font_atlas, m_render_context));

}


PlanetariumGUI::~PlanetariumGUI(){}


void PlanetariumGUI::onFramebufferSizeUpdate(){
    m_fb_update = true;
}


void PlanetariumGUI::updateVesselsText(const math::mat4& proj_mat, const math::mat4& view_mat){
    UNUSED(proj_mat);
    UNUSED(view_mat);
    /*
    //const VesselMap& active_vessels = m_asset_manager->m_active_vessels;
    const std::vector<struct planet_gui_data>& planets = m_system_gui_data.m_planets_data;
    wchar_t buff[256];

    for(uint i=0; i < planets.size(); i++){
        //const btVecror3& com = planets.at(i).m_planet->getCoM();
        const Planet& planets.at(i).m_planet;
        const btVector3& com = it->second->getCoM();


        math::vec4 pos(com.getX() / PLANETARIUM_SCALE_FACTOR,
                       com.getY() / PLANETARIUM_SCALE_FACTOR,
                       com.getZ() / PLANETARIUM_SCALE_FACTOR, 1.0f);
        math::vec4 pos_screen = proj_mat * view_mat * pos;

        if(pos_screen.v[2] > 0.0){
            pos_screen = ((pos_screen / pos_screen.v[3]) + 1. ) / 2.;

            mbstowcs(buff, it->second->getVesselName().c_str(), 256);
            m_main_text->addString(buff, pos_screen.v[0] * m_fb_width, 
                                   pos_screen.v[1] * m_fb_height + 5, 1.0f,
                                   STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);
        }

    }*/
}


bool comparator(const planet_gui_data& a, const planet_gui_data& b){
    return a.m_screen_dist > b.m_screen_dist;
}


void PlanetariumGUI::renderPlanets(const math::mat4& proj_mat, const math::mat4& view_mat){
    std::vector<struct planet_gui_data>& planets = m_system_gui_data.m_planets_data;
    wchar_t buff[256];


    for(uint i=0; i < planets.size(); i++){
        struct planet_gui_data& current = planets.at(i);
        const dmath::vec3& current_pos = current.m_planet_data->getPosition();

        math::vec4 pos(current_pos.v[0] / PLANETARIUM_SCALE_FACTOR,
                       current_pos.v[1] / PLANETARIUM_SCALE_FACTOR,
                       current_pos.v[2] / PLANETARIUM_SCALE_FACTOR, 1.0f);
        math::vec4 pos_screen = proj_mat * view_mat * pos;
        current.m_pos_screen = math::vec3(pos_screen);
        current.m_screen_dist = pos_screen.v[2];
    }

    std::sort(planets.begin(), planets.end(), comparator);
    m_system_gui_data.update_id_map();

    for(uint i=0; i < planets.size(); i++){
        const Planet* current = planets.at(i).m_planet_data;
        const dmath::vec3& current_pos = current->getPosition();

        math::vec4 pos(current_pos.v[0] / PLANETARIUM_SCALE_FACTOR,
                       current_pos.v[1] / PLANETARIUM_SCALE_FACTOR,
                       current_pos.v[2] / PLANETARIUM_SCALE_FACTOR, 1.0f);
        math::vec4 pos_screen = proj_mat * view_mat * pos;

        if(pos_screen.v[2] > 0.0){
            m_main_text->clearStrings();

            pos_screen = ((pos_screen / pos_screen.v[3]) + 1. ) / 2.;

            mbstowcs(buff, current->getName().c_str(), 256);
            float faded_c[4] = {0.85, 0.85, 0.85, 1 - m_target_fade};
            m_main_text->addString(buff, pos_screen.v[0] * m_fb_width, 
                                   pos_screen.v[1] * m_fb_height - 15, 1.0f,
                                   STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY,
                                   m_selected_planet == current->getId() ? faded_c : c);
            planets.at(i).m_planet_sprite.setAlpha(
                m_selected_planet == current->getId() ? 1 - m_target_fade : 1.0f);
            planets.at(i).m_planet_sprite.render(math::vec2(pos_screen.v[0] * m_fb_width, 
                                                            pos_screen.v[1] * m_fb_height));
            m_main_text->render();
        }
    }
}



void PlanetariumGUI::updateSceneText(const math::mat4& proj_mat, const math::mat4& view_mat){
    const PlanetarySystem* planetary_system = m_system_gui_data.m_planetary_system;
    const std::vector<struct planet_gui_data>& planets = m_system_gui_data.m_planets_data;
    uint selected_planet_idx;
    std::wostringstream woss;
    std::ostringstream oss;
    wchar_t buff[256];

    m_main_text->clearStrings();

    updateVesselsText(proj_mat, view_mat);

    oss << "System name: " << planetary_system->getSystemName();
    oss << "\nStar name: " << planetary_system->getStar().star_name;
    oss << "\nStar description: " << planetary_system->getStar().description;

    // selected it to index
    try{
        if(m_selected_planet)
            selected_planet_idx = m_system_gui_data.m_id_to_index.at(m_selected_planet);
        else
            selected_planet_idx = 0;
    }
    catch(const std::out_of_range& oor){
        std::cerr << "PlanetariumGUI::updateSceneText: wrong id value for selected planet: "
                  << m_selected_planet << " (what: " << oor.what() << ")" << std::endl;
        log("PlanetariumGUI::updateSceneText: wrong id value for selected planet: ",
            m_selected_planet, " (what: ", oor.what(), ")");
    }

    if(m_selected_planet && !m_freecam){
        try{
            const orbital_data& data = planets.at(selected_planet_idx).m_planet_data->getOrbitalData();
            oss << "\n\nSelected object: " << planets.at(selected_planet_idx).m_planet_data->getName();

            mbstowcs(buff, oss.str().c_str(), 256);
            woss << buff << std::fixed << std::setprecision(2);
            m_main_text->addString(woss.str().c_str(), 10, 15, 1.0f,
                                   STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT, c);

            woss.str(L"");
            woss.clear();

            double speed = dmath::length(data.pos - data.pos_prev) / m_delta_t;
            woss << L"\nOrbital parameters (J2000 eliptic): ";
            woss << L"\nOrbital speed: " << speed << L"m/s";
            woss << L"\nEccentricity (e): " << data.e;
            woss << L"\nSemi major axis (a): " << data.a << "AU";
            woss << L"\nInclination (i): " << data.i * ONE_RAD_IN_DEG << L"º";
            woss << L"\nLongitude of the asciending node (Ω): " 
                 << data.W * ONE_RAD_IN_DEG<< L"º";
            
            // too many strings already...
            m_main_text->addString(woss.str().c_str(), 10, 95, 1.0f,
                                   STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT, c);

            woss.str(L"");
            woss.clear();

            woss << L"Argument of the periapsis (ω): " << data.w * ONE_RAD_IN_DEG 
                 << L"º" << L" (ϖ: " << data.p << L"º)";    
            woss << L"\nTrue anomaly (f): " << data.v * ONE_RAD_IN_DEG << L"º"
                 << L" (M: " << data.M << L"º, L: " << data.L << L"º)";

            woss << L"\nPeriod: " << data.period * 36525 << L" days (" << data.period * 100.
                 << L" years)";
            woss << L"\nPerigee: " << (1 - data.e) * data.a
                                       * AU_TO_METERS / 1000.0 << L"km";
            woss << L"\nApogee : " << (1 + data.e) * data.a
                                       * AU_TO_METERS / 1000.0 << L"km";

            woss << L"\n\nPhysical properties: ";
            woss << L"\nMass: " << std::scientific << data.m << "kg";

            m_main_text->addString(woss.str().c_str(), 10, 235, 1.0f,
                                   STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT, c);
        }
        catch(const std::out_of_range& oor){
            std::cerr << "PlanetariumGUI::updateSceneText: wrong index value for selected planet: "
                      << selected_planet_idx << " (what: " << oor.what() << ")" << std::endl;
            log("PlanetariumGUI::updateSceneText: wrong index value for selected planet: ",
                selected_planet_idx, " (what: ", oor.what(), ")");
        }
    }

    woss.str(L"");
    woss.clear();
    time_t current_time = (time_t)m_physics->getCurrentTime() + SECS_FROM_UNIX_TO_J2000;
    mbstowcs(buff, ctime(&current_time), 256);
    m_main_text->addString(buff, 25, 50, 1.0f,
                           STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_RIGHT, c);

    woss << L"delta_t: " << m_delta_t << L"ms (" << m_delta_t / 36000.0
         << L" hours, x" << long(m_delta_t / (1. / 60.)) << L")";
    m_main_text->addString(woss.str().c_str(), 25, 35, 1.0f,
                           STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_RIGHT, c);
}



void PlanetariumGUI::render(){
    if(m_fb_update){
        m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
        m_main_text->onFramebufferSizeUpdate(m_fb_width, m_fb_height);

        m_main_text->clearStrings();
        m_fb_update = false;
    }
    const math::mat4& proj_mat = m_camera->getProjMatrix();
    const math::mat4 view_mat = m_camera->getViewMatrix();
    renderPlanets(proj_mat, view_mat);
    updateSceneText(proj_mat, view_mat);
    m_main_text->render();
}


int PlanetariumGUI::update(){
    return PLANETARIUM_ACTION_NONE;
}


void PlanetariumGUI::setSimulationDeltaT(double delta_t){
    m_delta_t = delta_t;
}


void PlanetariumGUI::setSelectedPlanet(std::uint32_t planet_id){
    m_selected_planet = planet_id;
}


void PlanetariumGUI::buildSystemGUIData(){
    const PlanetarySystem* planetary_system = m_asset_manager->m_planetary_system.get();
    const planet_map& planets = planetary_system->getPlanets();
    planet_map::const_iterator it;

    m_system_gui_data.m_planetary_system = planetary_system;

    for(it=planets.begin();it!=planets.end();it++){
        const Planet* current = it->second.get();
        Sprite sprite(m_render_context, math::vec2(.0f, .0f), SPRITE_DRAW_ABSOLUTE,
                      current->getThumbnailPath(), 24.0f);
        m_system_gui_data.m_planets_data.emplace_back(current, std::move(sprite));
    }
    m_system_gui_data.update_id_map();
}


void PlanetariumGUI::setFreecam(bool freecam){
    m_freecam = freecam;
}


void PlanetariumGUI::setTargetFade(float value){
    m_target_fade = value;
}


void PlanetariumGUI::renderImGUI(){
    static bool window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar
                             | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoResize
                             | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove
                             | ImGuiWindowFlags_NoBackground;
    int fb_x, fb_y;
    m_app->getWindowHandler()->getFramebufferSize(fb_x, fb_y);

    // upper right menu
    ImGui::SetNextWindowSize(ImVec2(1000, 39));
    ImGui::SetNextWindowPos(ImVec2(fb_x - 235, 0));
    ImGui::Begin("Planetarium settings", nullptr, window_flags);
    if (ImGui::Button("Settings"))
        m_show_settings = !m_show_settings;
    ImGui::End();

    if(m_show_settings){
        ImGui::Begin("Predictor settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        ImGui::Text("Predictor configuration");
        ImGui::InputInt("Predictor steps", &m_predictor_steps);
        ImGui::InputFloat("Predictor period (days)", &m_predictor_period);

        if(m_predictor_steps <= 0){
            m_predictor_steps = 1;
        }
        ImGui::End();
    }


}