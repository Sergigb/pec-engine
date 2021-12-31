#include <unordered_map>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <algorithm>

#include "Planetarium.hpp"
#include "core/log.hpp"
#include "core/utils/utils.hpp"
#include "core/maths_funcs.hpp"
#include "core/RenderContext.hpp"
#include "core/Camera.hpp"
#include "core/Input.hpp"
#include "core/WindowHandler.hpp"
#include "core/Player.hpp"
#include "core/Physics.hpp"
#include "core/Frustum.hpp"
#include "core/AssetManager.hpp"
#include "GUI/FontAtlas.hpp"
#include "GUI/Text2D.hpp"
#include "assets/PlanetarySystem.hpp"
#include "assets/Planet.hpp"


Planetarium::Planetarium() : BaseApp(){
    init();
}


Planetarium::Planetarium(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


bool comparator(const Planet* a, const Planet* b){
    return a->getOrbitalData().semi_major_axis_0 < b->getOrbitalData().semi_major_axis_0;
}


void Planetarium::init(){
    m_window_handler->setWindowTitle("Planetarium");
    m_seconds_since_j2000 = 0.0;

    m_asset_manager->loadStarSystem();

    m_delta_t = (1000. / 60.0) / 1000.0;  // in ms

    planet_map::const_iterator it;
    const planet_map& planets = m_asset_manager->m_planetary_system->getPlanets();

    for(it=planets.begin();it!=planets.end();it++)
        m_bodies.emplace_back(it->second.get());
    std::sort(m_bodies.begin(), m_bodies.end(), comparator);
    m_pick = 2;
    m_player->setSelectedPlanet(m_bodies.at(m_pick)->getId());

    initGl();

    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);

    int fb_width, fb_height;
    m_window_handler->getFramebufferSize(fb_width, fb_height);

    m_text.reset(new Text2D(fb_width, fb_height, {0.f, 1.f, 1.f}, 
                            m_def_font_atlas.get(), m_render_context.get()));
    m_text2.reset(new Text2D(fb_width, fb_height, {0.f, 1.f, 0.f}, 
                            m_def_font_atlas.get(), m_render_context.get()));
}


Planetarium::~Planetarium(){
}


GLuint color_location, proj_location, view_location; // change me :)

void Planetarium::initGl(){
    color_location = m_render_context->getUniformLocation(SHADER_DEBUG, "line_color");
    proj_location = m_render_context->getUniformLocation(SHADER_DEBUG, "proj");
    view_location = m_render_context->getUniformLocation(SHADER_DEBUG, "view");
}


void Planetarium::renderOrbits(){
    planet_map::const_iterator it;
    const planet_map& planets = m_asset_manager->m_planetary_system->getPlanets();

    m_render_context->useProgram(SHADER_DEBUG);
    glUniformMatrix4fv(view_location, 1, GL_FALSE, m_camera->getViewMatrix().m);
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, m_camera->getProjMatrix().m);

    for(it=planets.begin();it!=planets.end();it++){
        Planet* current = it->second.get();

        if(current == m_bodies.at(m_pick))
            glUniform3f(color_location, 0.0, 1.0, 0.0);
        else
            glUniform3f(color_location, 1.0, 0.0, 0.0);

        current->renderOrbit();
    }
}


void Planetarium::processInput(){
    double scx, scy;
    m_input->getScroll(scx, scy);

    if(m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & INPUT_KEY_REPEAT)
        m_delta_t *= scy == 0.f ? 1.f : (scy < 0.f ? .1f : 10.f);
    
    else{
        if(m_player->getPlanetariumFreecam())
            m_camera->setSpeed(m_camera->getSpeed() * (scy == 0.f ? 1.f : (scy < 0.f ? .1f : 10.f)));
        else
            m_camera->incrementOrbitalCamDistance(-scy * 5.0);
    }

    if(m_input->pressed_keys[GLFW_KEY_R] & INPUT_KEY_RELEASE){
        m_render_context->reloadShaders();
        std::cout << "Shaders reloaded" << std::endl;
    }

    if(m_input->pressed_keys[GLFW_KEY_ESCAPE] & INPUT_KEY_RELEASE){
        glfwSetWindowShouldClose(m_window_handler->getWindow(), GLFW_TRUE);
    }

    if(m_input->pressed_keys[GLFW_KEY_F12] & INPUT_KEY_RELEASE){
        m_render_context->toggleDebugOverlay();
    }

    if(m_input->pressed_keys[GLFW_KEY_TAB] & INPUT_KEY_DOWN){
        m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & INPUT_KEY_REPEAT ? m_pick-- : m_pick++;
        if(m_pick >= m_bodies.size())
            m_pick = 0;
        m_player->setSelectedPlanet(m_bodies.at(m_pick)->getId());
    }
}


void Planetarium::logic(){
    double centuries_since_j2000 = 0.0;
    time_t current_time;
    wchar_t buff[256];
    std::wostringstream woss;

    processInput();

    centuries_since_j2000 = m_seconds_since_j2000 / SECONDS_IN_A_CENTURY;
    m_asset_manager->m_planetary_system->updateOrbitalElements(centuries_since_j2000);
    updateSceneText();

    m_text2->clearStrings();

    current_time = (time_t)m_seconds_since_j2000 + SECS_FROM_UNIX_TO_J2000;
    mbstowcs(buff, ctime(&current_time), 256);
    m_text2->addString(buff, 25, 50, 1.0f,
                      STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_RIGHT);

    woss << L"delta_t: " << m_delta_t << L"ms (" << m_delta_t / 36000.0
         << L" hours, x" << long(m_delta_t / (1. / 60.)) << L")";
    m_text2->addString(woss.str().c_str(), 25, 35, 1.0f,
                      STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_RIGHT);

    m_seconds_since_j2000 += m_delta_t;

    m_asset_manager->m_planetary_system->updateRenderBuffers(centuries_since_j2000);
    renderOrbits();

    m_text->render();
    m_text2->render();
}


void Planetarium::updateSceneText(){
    planet_map::const_iterator it;
    const planet_map& planets = m_asset_manager->m_planetary_system->getPlanets();
    const math::mat4& proj_mat = m_camera->getProjMatrix();
    const math::mat4 view_mat = m_camera->getViewMatrix();
    int w, h;
    std::wostringstream woss;
    std::ostringstream oss;
    wchar_t buff[256];

    m_window_handler->getFramebufferSize(w, h);

    m_text->clearStrings();
    m_text->onFramebufferSizeUpdate(w, h);  // this is not updated or notified by anyone in RenderContext so we do it here

    for(it=planets.begin();it!=planets.end();it++){
        const Planet* current = it->second.get();

        math::vec4 pos(current->getPosition().v[0] / PLANETARIUM_DEF_SCALE_FACTOR,
                       current->getPosition().v[1] / PLANETARIUM_DEF_SCALE_FACTOR,
                       current->getPosition().v[2] / PLANETARIUM_DEF_SCALE_FACTOR, 1.0f);
        math::vec4 pos_screen = proj_mat * view_mat * pos;
        pos_screen = ((pos_screen / pos_screen.v[3]) + 1. ) / 2.; // there's something wrong here

        mbstowcs(buff, current->getName().c_str(), 256);
        m_text->addString(buff, pos_screen.v[0] * w, pos_screen.v[1] * h + 5, 1.0f,
                          STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);
    }

    oss << "System name: " << m_asset_manager->m_planetary_system->getSystemName();
    oss << "\nStar name: " << m_asset_manager->m_planetary_system->getStar().star_name;
    oss << "\nStar description: " << m_asset_manager->m_planetary_system->getStar().description;

    const orbital_data& data = m_bodies.at(m_pick)->getOrbitalData();
    double speed = dmath::length(data.pos - data.pos_prev) / m_delta_t;
    oss << "\n\nSelected object: " << m_bodies.at(m_pick)->getName();
    mbstowcs(buff, oss.str().c_str(), 256);

    woss << buff << std::fixed << std::setprecision(2);
    m_text->addString(woss.str().c_str(), 10, 15, 1.0f,
                      STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);

    woss.str(L"");
    woss.clear();

    woss << L"\nOrbital parameters (J2000 eliptic): ";
    woss << L"\nOrbital speed: " << speed << L"m/s";
    woss << L"\nEccentricity (e): " << data.eccentricity;
    woss << L"\nSemi major axis (a): " << data.semi_major_axis << "AU";
    woss << L"\nInclination (i): " << data.inclination * ONE_RAD_IN_DEG << L"º";
    woss << L"\nLongitude of the asciending node (Ω): " << data.long_asc_node * ONE_RAD_IN_DEG<< L"º";
    
    // too many strings already...
    m_text->addString(woss.str().c_str(), 10, 95, 1.0f,
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
    woss << L"\nMass: " << std::scientific << data.mass << "kg";

    m_text->addString(woss.str().c_str(), 10, 235, 1.0f,
                      STRING_DRAW_ABSOLUTE_TL, STRING_ALIGN_RIGHT);    
}


void Planetarium::run(){
    m_camera->setCameraPosition(dmath::vec3(500.0, 1000.0, 0.0));
    m_camera->setSpeed(50.0f);
    m_camera->createProjMat(.001, 100000.0, 67.0, 1.0);
    m_player->setBehaviour(PLAYER_BEHAVIOUR_PLANETARIUM);

    //glDisable(GL_CULL_FACE);

    m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));

    while(!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
        m_player->updatePlanetarium();

        m_render_context->contextUpdatePlanetRenderer(); // works in here too :)
        glClearColor(0.f, 0.f, 0.f, 0.f); // loook at the todo list
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        logic();

        glfwSwapBuffers(m_window_handler->getWindow());
    }

    m_window_handler->terminate();
}

