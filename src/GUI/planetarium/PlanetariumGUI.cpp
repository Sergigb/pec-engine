#include <iomanip>

#include "PlanetariumGUI.hpp"
#include "../FontAtlas.hpp"
#include "../Text2D.hpp"
#include "../../core/RenderContext.hpp"
#include "../../core/log.hpp"
#include "../../core/Camera.hpp"
#include "../../assets/PlanetarySystem.hpp"



PlanetariumGUI::PlanetariumGUI(const FontAtlas* atlas, const RenderContext* render_context, const Camera* camera){
    m_font_atlas = atlas;
    m_render_context = render_context;
    m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
    m_camera = camera;
    m_fb_update = true;
    m_planetary_system = nullptr;
    m_delta_t = 1 / 60.;

    m_main_text.reset(new Text2D(m_fb_width, m_fb_height, color{0.85, 0.85, 0.85},
                      m_font_atlas, render_context));

}


PlanetariumGUI::~PlanetariumGUI(){}


void PlanetariumGUI::onFramebufferSizeUpdate(){
    m_fb_update = true;
}


void PlanetariumGUI::updateSceneText(){
    planet_map::const_iterator it;
    const planet_map& planets = m_planetary_system->getPlanets();
    const math::mat4& proj_mat = m_camera->getProjMatrix();
    const math::mat4 view_mat = m_camera->getViewMatrix();
    std::wostringstream woss;
    std::ostringstream oss;
    wchar_t buff[256];

    m_main_text->clearStrings();

    for(it=planets.begin();it!=planets.end();it++){
        const Planet* current = it->second.get();

        math::vec4 pos(current->getPosition().v[0] / 1e10,
                       current->getPosition().v[1] / 1e10,
                       current->getPosition().v[2] / 1e10, 1.0f);
        math::vec4 pos_screen = proj_mat * view_mat * pos;
        pos_screen = ((pos_screen / pos_screen.v[3]) + 1. ) / 2.; // there's something wrong here

        mbstowcs(buff, current->getName().c_str(), 256);
        m_main_text->addString(buff, pos_screen.v[0] * m_fb_width, 
                               pos_screen.v[1] * m_fb_height + 5, 1.0f,
                               STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);
    }

    oss << "System name: " << m_planetary_system->getSystemName();
    oss << "\nStar name: " << m_planetary_system->getStar().star_name;
    oss << "\nStar description: " << m_planetary_system->getStar().description;

    //const orbital_data& data = m_bodies.at(m_pick)->getOrbitalData();
    //double speed = dmath::length(data.pos - data.pos_prev) / m_delta_t;
   // oss << "\n\nSelected object: " << m_bodies.at(m_pick)->getName();
    mbstowcs(buff, oss.str().c_str(), 256);

    woss << buff << std::fixed << std::setprecision(2);
    m_main_text->addString(woss.str().c_str(), 10, 15, 1.0f,
                      STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);

    /*woss.str(L"");
    woss.clear();

    woss << L"\nOrbital parameters (J2000 eliptic): ";
    woss << L"\nOrbital speed: " << speed << L"m/s";
    woss << L"\nEccentricity (e): " << data.eccentricity;
    woss << L"\nSemi major axis (a): " << data.semi_major_axis << "AU";
    woss << L"\nInclination (i): " << data.inclination * ONE_RAD_IN_DEG << L"º";
    woss << L"\nLongitude of the asciending node (Ω): " << data.long_asc_node * ONE_RAD_IN_DEG<< L"º";
    
    // too many strings already...
    m_main_text->addString(woss.str().c_str(), 10, 95, 1.0f,
                           STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);

    woss.str(L"");
    woss.clear();

    woss << L"Argument of the periapsis (ω): " << data.arg_periapsis * ONE_RAD_IN_DEG << L"º"
         << L" (ϖ: " << data.longitude_perigee << L"º)";    
    woss << L"\nTrue anomaly (f): " << data.true_anomaly * ONE_RAD_IN_DEG << L"º"
         << L" (M: " << data.mean_anomaly << L"º, L: " << data.mean_longitude << L"º)";

    woss << L"\nPeriod: " << data.period * 36525 << L" days (" << data.period * 100. << L" years)";
    woss << L"\nPerigee: " << (1 - data.eccentricity) * data.semi_major_axis * AU_TO_METERS / 1000.0 << L"km";
    woss << L"\nApogee : " << (1 + data.eccentricity) * data.semi_major_axis * AU_TO_METERS / 1000.0 << L"km";

    woss << L"\n\nPhysical properties: ";
    woss << L"\nMass: " << std::scientific << data.mass << "kg";*/

    m_main_text->addString(woss.str().c_str(), 10, 235, 1.0f,
                           STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);    
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


void PlanetariumGUI::setPlanetarySystem(const PlanetarySystem* planetary_system){
    m_planetary_system = planetary_system;
}


void PlanetariumGUI::setSimulationDeltaT(double delta_t){
    m_delta_t = delta_t;
}
