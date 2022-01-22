#include <iomanip>

#include "PlanetariumGUI.hpp"
#include "../FontAtlas.hpp"
#include "../Text2D.hpp"
#include "../../core/RenderContext.hpp"
#include "../../core/log.hpp"
#include "../../core/Camera.hpp"
#include "../../core/Physics.hpp"
#include "../../core/AssetManager.hpp"
#include "../../assets/PlanetarySystem.hpp"
#include "../../assets/Vessel.hpp"



PlanetariumGUI::PlanetariumGUI(const FontAtlas* atlas, const RenderContext* render_context,
                               const Camera* camera, const Physics* physics,
                               const AssetManager* asset_manager){
    m_font_atlas = atlas;
    m_render_context = render_context;
    m_physics = physics;
    m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
    m_camera = camera;
    m_fb_update = true;
    m_delta_t = 1 / 60.;
    m_selected_planet = 0;
    m_asset_manager = asset_manager;

    m_main_text.reset(new Text2D(m_fb_width, m_fb_height, color{0.0, 1., 0.0},
                      m_font_atlas, render_context));

}


PlanetariumGUI::~PlanetariumGUI(){}


void PlanetariumGUI::onFramebufferSizeUpdate(){
    m_fb_update = true;
}


void PlanetariumGUI::updateVesselsText(const math::mat4& proj_mat, const math::mat4& view_mat){
    const VesselMap& active_vessels = m_asset_manager->m_active_vessels;
    VesselMap::const_iterator it;
    wchar_t buff[256];

    for(it=active_vessels.begin(); it!=active_vessels.end(); it++){
        const btVector3& com = it->second->getCoM();

        math::vec4 pos(com.getX() / 1e10,
                       com.getY() / 1e10,
                       com.getZ() / 1e10, 1.0f);
        math::vec4 pos_screen = proj_mat * view_mat * pos;

        if(pos_screen.v[2] > 0.0){
            pos_screen = ((pos_screen / pos_screen.v[3]) + 1. ) / 2.;

            mbstowcs(buff, it->second->getVesselName().c_str(), 256);
            m_main_text->addString(buff, pos_screen.v[0] * m_fb_width, 
                                   pos_screen.v[1] * m_fb_height + 5, 1.0f,
                                   STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);
        }

    }
}


void PlanetariumGUI::updatePlanetsText(const math::mat4& proj_mat, const math::mat4& view_mat){
    const PlanetarySystem* planetary_system = m_asset_manager->m_planetary_system.get();
    const planet_map& planets = planetary_system->getPlanets();
    planet_map::const_iterator it;
    wchar_t buff[256];

    for(it=planets.begin();it!=planets.end();it++){
        const Planet* current = it->second.get();
        const dmath::vec3& current_pos = current->getPosition();

        math::vec4 pos(current_pos.v[0] / 1e10,
                       current_pos.v[1] / 1e10,
                       current_pos.v[2] / 1e10, 1.0f);
        math::vec4 pos_screen = proj_mat * view_mat * pos;

        if(pos_screen.v[2] > 0.0){
            pos_screen = ((pos_screen / pos_screen.v[3]) + 1. ) / 2.;

            mbstowcs(buff, current->getName().c_str(), 256);
            m_main_text->addString(buff, pos_screen.v[0] * m_fb_width, 
                                   pos_screen.v[1] * m_fb_height + 5, 1.0f,
                                   STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);
        }
    }
}


void PlanetariumGUI::updateSceneText(){
    const PlanetarySystem* planetary_system = m_asset_manager->m_planetary_system.get();
    const planet_map& planets = planetary_system->getPlanets();
    const math::mat4& proj_mat = m_camera->getProjMatrix();
    const math::mat4 view_mat = m_camera->getViewMatrix();

    std::wostringstream woss;
    std::ostringstream oss;
    wchar_t buff[256];

    m_main_text->clearStrings();

    updateVesselsText(proj_mat, view_mat);
    updatePlanetsText(proj_mat, view_mat);

    oss << "System name: " << planetary_system->getSystemName();
    oss << "\nStar name: " << planetary_system->getStar().star_name;
    oss << "\nStar description: " << planetary_system->getStar().description;

    if(m_selected_planet){
        try{
            const orbital_data& data = planets.at(m_selected_planet)->getOrbitalData();
            oss << "\n\nSelected object: " << planets.at(m_selected_planet)->getName();

            mbstowcs(buff, oss.str().c_str(), 256);
            woss << buff << std::fixed << std::setprecision(2);
            m_main_text->addString(woss.str().c_str(), 10, 15, 1.0f,
                                   STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);

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
                                   STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);

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
                                   STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);
        }
        catch (const std::out_of_range& oor) {
            std::cerr << "PlanetariumGUI::updateSceneText: wrong id value for selected planet: " 
                      << m_selected_planet << " (what: " << oor.what() << ")" << std::endl;
            log("PlanetariumGUI::updateSceneText: wrong id value for selected planet: ",
                m_selected_planet, " (what: ", oor.what(), ")");
        }
    }

    woss.str(L"");
    woss.clear();
    time_t current_time = (time_t)m_physics->getCurrentTime() + SECS_FROM_UNIX_TO_J2000;
    mbstowcs(buff, ctime(&current_time), 256);
    m_main_text->addString(buff, 25, 50, 1.0f,
                           STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_RIGHT);

    woss << L"delta_t: " << m_delta_t << L"ms (" << m_delta_t / 36000.0
         << L" hours, x" << long(m_delta_t / (1. / 60.)) << L")";
    m_main_text->addString(woss.str().c_str(), 25, 35, 1.0f,
                           STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_RIGHT);
}



void PlanetariumGUI::render(){
    if(m_fb_update){
        m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
        m_main_text->onFramebufferSizeUpdate(m_fb_width, m_fb_height);

        m_main_text->clearStrings();
        m_fb_update = false;
    }
    updateSceneText();
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
