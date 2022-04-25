#include <unordered_map>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <algorithm>
#include <cmath>

#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

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
#include "GUI/planetarium/PlanetariumGUI.hpp"
#include "assets/PlanetarySystem.hpp"
#include "assets/Planet.hpp"


Planetarium::Planetarium() : BaseApp(){
    init();
}


Planetarium::Planetarium(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


bool comparator(const Planet* a, const Planet* b){
    return a->getOrbitalData().a_0 < b->getOrbitalData().a_0;
}


void Planetarium::init(){
    m_window_handler->setWindowTitle("Planetarium");
    m_seconds_since_j2000 = 0.0;

    m_asset_manager->loadStarSystem();

    m_delta_t = REAL_TIME_DT;  // in ms
    m_dt_multiplier = 1.0;

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

    m_planetarium_gui.reset(new PlanetariumGUI(m_def_font_atlas.get(), m_render_context.get(),
                                               m_camera.get(), m_physics.get(),
                                               m_asset_manager.get()));


    // particles
    m_particles.emplace_back(dmath::vec3(180000000000.0, 0.0, 0.0),
                             dmath::vec3(0.0, 0.0, 27154.0),
                             1.0);
    m_predictor_steps = 300;
    m_predictor_period = 365;
}


Planetarium::~Planetarium(){
}


GLuint color_location, proj_location, view_location; // change me :)

void Planetarium::initGl(){
    color_location = m_render_context->getUniformLocation(SHADER_DEBUG, "line_color");
    proj_location = m_render_context->getUniformLocation(SHADER_DEBUG, "proj");
    view_location = m_render_context->getUniformLocation(SHADER_DEBUG, "view");

    // init temp buffers
    glGenVertexArrays(1, &m_pred_vao);
    m_render_context->bindVao(m_pred_vao);

    glGenBuffers(1, &m_pred_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_pred_vbo_vert);
    glVertexAttribPointer(0, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_pred_vbo_ind);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pred_vbo_ind);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_INT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
}


void Planetarium::renderParticles(){
    const math::mat4& proj_mat = m_camera->getProjMatrix();
    const math::mat4 view_mat = m_camera->getViewMatrix();
    int fb_width, fb_height;
    m_window_handler->getFramebufferSize(fb_width, fb_height);

    m_text->clearStrings();
    for(uint i=0; i<m_particles.size(); i++){
        const dmath::vec3& current_pos = m_particles.at(i).origin;
        math::vec4 pos(current_pos.v[0] / 1e10,
                       current_pos.v[1] / 1e10,
                       current_pos.v[2] / 1e10, 1.0f);
        math::vec4 pos_screen = proj_mat * view_mat * pos;

        if(pos_screen.v[2] > 0.0){
            pos_screen = ((pos_screen / pos_screen.v[3]) + 1. ) / 2.;

            m_text->addString(L"particle", pos_screen.v[0] * fb_width,
                              pos_screen.v[1] * fb_height + 5, 1.0f,
                              STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);
        }
    }
    m_text->render();
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
            glUniform3f(color_location, 1.0, 0.0, 0.0);
        else
            glUniform3f(color_location, 0.1, .75, 0.25);

        current->renderOrbit();
    }

    glUniform3f(color_location, 0.0, .75, 0.75);
    m_render_context->bindVao(m_pred_vao);
    glDrawElements(GL_LINES, m_predictor_steps * 2, GL_UNSIGNED_INT, NULL);
}


void Planetarium::processInput(){
    double scx, scy;
    
    ImGuiIO& io = ImGui::GetIO();
    if(io.WantCaptureMouse || io.WantCaptureKeyboard){
        return;
    }

    m_input->getScroll(scx, scy);

    if(m_player->getPlanetariumFreecam())
        m_camera->setSpeed(m_camera->getSpeed() * (scy == 0.f ? 1.f : (scy < 0.f ? .1f : 10.f)));
    else
        m_camera->incrementOrbitalCamDistance(-scy * 5.0);


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


int which_particle = 0;
void Planetarium::updatePredictionBuffer(){
    planet_map::const_iterator it;
    const planet_map& planets = m_asset_manager->m_planetary_system->getPlanets();
    double star_mass = m_asset_manager->m_planetary_system->getStar().mass;
    dmath::vec3 planet_origin;

    std::unique_ptr<GLfloat[]> vertex_buffer;
    std::unique_ptr<GLuint[]> index_buffer;

    vertex_buffer.reset(new GLfloat[3 * m_predictor_steps]);
    index_buffer.reset(new GLuint[2 * m_predictor_steps]);

    m_render_context->bindVao(m_pred_vao);

    particle selected_particle = m_particles.at(which_particle);

    double e, W, w, inc, a, L, p, M, v;
    double time = m_seconds_since_j2000 / SECONDS_IN_A_CENTURY;
    double predictor_delta_t_secs = (m_predictor_period * 24 * 60 * 60) / m_predictor_steps;
    double predictor_delta_t_cent = predictor_delta_t_secs / SECONDS_IN_A_CENTURY;

    // only once per prediction, probably doesn't affect too much the precision
    for(it=planets.begin();it!=planets.end();it++){
        const orbital_data& data = it->second->getOrbitalData();
        a = data.a_0 + data.a_d * time;
        e = data.e_0 + data.e_d * time;
        inc = data.i_0 + data.i_d * time;
        L = data.L_0 + data.L_d * time;
        p = data.p_0 + data.p_d * time;
        W = data.W_0 + data.W_d * time;


        M = L - p;
        w = p - W;

        double E = M;
        double ecc_d = 10.8008135;
        int iter = 0;
        while(std::abs(ecc_d) > 1e-6 && iter < MAX_SOLVER_ITER){
            ecc_d = (E - e * std::sin(E) - M) / (1 - e * std::cos(E));
            E -= ecc_d;
            iter++;
        }

        v = 2 * std::atan(std::sqrt((1 + e) / (1 - e)) * std::tan(E / 2));

        double rad = a * (1 - e * std::cos(E)) * AU_TO_METERS;

        planet_origin.v[0] = rad * (std::cos(W) * std::cos(w + v) -
                             std::sin(W) * std::sin(w + v) * std::cos(inc));
        planet_origin.v[1] = rad * (std::sin(inc) * std::sin(w + v));
        planet_origin.v[2] = rad * (std::sin(W) * std::cos(w + v) +
                             std::cos(W) * std::sin(w + v) *std::cos(inc));
    }

    for(int i=0; i < m_predictor_steps; i++){
        vertex_buffer[i * 3] = selected_particle.origin.v[0] / 1e10;
        vertex_buffer[i * 3 + 1] = selected_particle.origin.v[1] / 1e10;
        vertex_buffer[i * 3 + 2] = selected_particle.origin.v[2] / 1e10;

        // relative
        //vertex_buffer[i * 3] = (selected_particle.origin.v[0] - m_bodies.at(2)->getOrbitalData().pos.v[0]) / 1e10;
        //vertex_buffer[i * 3 + 1] = (selected_particle.origin.v[1] - m_bodies.at(2)->getOrbitalData().pos.v[1]) / 1e10;
        //vertex_buffer[i * 3 + 2] = (selected_particle.origin.v[2] - m_bodies.at(2)->getOrbitalData().pos.v[2]) / 1e10;

        index_buffer[i * 2] = i;
        index_buffer[i * 2 + 1] = i + 1;

        time += predictor_delta_t_cent;

        for(it=planets.begin();it!=planets.end();it++){
            const orbital_data& data = it->second->getOrbitalData();

            // apply gravity
            double Rh = dmath::distance(planet_origin, selected_particle.origin);

            double acceleration = GRAVITATIONAL_CONSTANT * (data.m / (Rh*Rh));
            dmath::vec3 f = dmath::normalise(planet_origin - selected_particle.origin)
                                             * acceleration * selected_particle.mass;
            selected_particle.total_force += f;
        }
        // star
        double Rh = dmath::length(selected_particle.origin);
        double acceleration = GRAVITATIONAL_CONSTANT * (star_mass / (Rh*Rh));
        dmath::vec3 f = dmath::normalise(dmath::vec3(-selected_particle.origin.v[0],
                                                     -selected_particle.origin.v[1],
                                                     -selected_particle.origin.v[2])) * acceleration * selected_particle.mass;
        selected_particle.total_force += f;

        // solve
        solverSymplecticEuler(selected_particle, predictor_delta_t_secs);
    }
    index_buffer[(2 * m_predictor_steps) - 1] = m_predictor_steps - 1;

    glBindBuffer(GL_ARRAY_BUFFER, m_pred_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 3 * m_predictor_steps * sizeof(GLfloat), vertex_buffer.get(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pred_vbo_ind);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * m_predictor_steps * sizeof(GLuint), index_buffer.get(), GL_STATIC_DRAW);
}


math::vec3 vel_select(0.0, 0.0, 0.0);
math::vec3 pos_select(0.0, 0.0, 0.0);
uint select_relative_to = 0;
bool select_relative = false;

void Planetarium::render(){
    renderOrbits();
    m_planetarium_gui->render();
    renderParticles();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    
    ImGui::Begin("Planetarium settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::InputDouble("Delta T exp. multiplier", &m_dt_multiplier, 1.0, 1.0);
    m_delta_t = REAL_TIME_DT * std::pow(10, m_dt_multiplier);

    if(m_dt_multiplier < 0){
        m_dt_multiplier = 1.0;
    }

    ImGui::Text("Selected body");
    if(ImGui::BeginCombo("##selected_body", m_bodies.at(m_pick)->getName().c_str(), 0)){
        for(uint i=0; i < m_bodies.size(); i++){
            bool is_selected = m_pick == i;
            if(ImGui::Selectable(m_bodies.at(i)->getName().c_str(), is_selected)){
                m_pick = i;
                m_player->setSelectedPlanet(m_bodies.at(m_pick)->getId());
            }
            if(is_selected){
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Separator();
    ImGui::Text("Predictor configuration");
    ImGui::InputInt("Predictor steps", &m_predictor_steps);
    ImGui::InputInt("Predictor period (days)", &m_predictor_period);

    if(m_predictor_steps <= 0){
        m_predictor_steps = 1;
    }

    ImGui::Separator();
    ImGui::Text("Particle");
    ImGui::InputFloat3("Position", pos_select.v);
    ImGui::InputFloat3("Velocity", vel_select.v);

    ImGui::Text("Target");
    if(ImGui::BeginCombo("##rel_sel", m_bodies.at(select_relative_to)->getName().c_str(), 0)){
        for(uint i=0; i < m_bodies.size(); i++){
            bool is_selected = select_relative_to == i;
            if(ImGui::Selectable(m_bodies.at(i)->getName().c_str(), is_selected)){
                select_relative_to = i;
            }
            if(is_selected){
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Relative", &select_relative);

    if(ImGui::Button("Set")){
        m_particles.at(which_particle).velocity.v[0] = vel_select.v[0];
        m_particles.at(which_particle).velocity.v[1] = vel_select.v[1];
        m_particles.at(which_particle).velocity.v[2] = vel_select.v[2];

        m_particles.at(which_particle).origin.v[0] = pos_select.v[0];
        m_particles.at(which_particle).origin.v[1] = pos_select.v[1];
        m_particles.at(which_particle).origin.v[2] = pos_select.v[2];

        if(select_relative){
            m_particles.at(which_particle).origin +=
                m_bodies.at(select_relative_to)->getOrbitalData().pos;
        }
    }

    ImGui::End();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


void Planetarium::applyGravity(){
    planet_map::const_iterator it;
    const planet_map& planets = m_asset_manager->m_planetary_system->getPlanets();
    double star_mass = m_asset_manager->m_planetary_system->getStar().mass;

    for(uint i=0; i < m_particles.size(); i++){
        const dmath::vec3& object_origin = m_particles.at(i).origin;

        for(it=planets.begin();it!=planets.end();it++){
            const orbital_data& data = it->second->getOrbitalData();
            double Rh = dmath::distance(data.pos, object_origin);

            double acceleration = GRAVITATIONAL_CONSTANT * (data.m / (Rh*Rh));
            dmath::vec3 f = dmath::normalise(data.pos - object_origin) * acceleration
                                             * m_particles.at(i).mass;
            m_particles.at(i).total_force += f;
        }

        double Rh = dmath::length(object_origin);
        double acceleration = GRAVITATIONAL_CONSTANT * (star_mass / (Rh*Rh));
        dmath::vec3 f = dmath::normalise(dmath::vec3(-object_origin.v[0],
                                                     -object_origin.v[1],
                                                     -object_origin.v[2])) * acceleration * m_particles.at(i).mass;
        m_particles.at(i).total_force += f;
    }
}


inline void Planetarium::solverExplicitEuler(struct particle& p, double delta_t){
    dmath::vec3 acceleration = p.total_force / p.mass;
        
    dmath::vec3 velocity_tp1 = p.velocity + acceleration * delta_t;
    p.origin = p.origin + p.velocity * delta_t;

    p.velocity = velocity_tp1;
    p.total_force.v[0] = 0.0;
    p.total_force.v[1] = 0.0;
    p.total_force.v[2] = 0.0;
}


inline void Planetarium::solverSymplecticEuler(struct particle& p, double delta_t){
    dmath::vec3 acceleration = p.total_force / p.mass;

    dmath::vec3 velocity_tp1 = p.velocity + acceleration * delta_t;
    p.origin = p.origin + velocity_tp1 * delta_t;

    p.velocity = velocity_tp1;
    p.total_force.v[0] = 0.0;
    p.total_force.v[1] = 0.0;
    p.total_force.v[2] = 0.0;
}


void Planetarium::logic(){
    double centuries_since_j2000 = 0.0;

    processInput();

    centuries_since_j2000 = m_seconds_since_j2000 / SECONDS_IN_A_CENTURY;
    m_asset_manager->m_planetary_system->updateOrbitalElements(centuries_since_j2000);

    applyGravity();

    for(uint i=0; i<m_particles.size(); i++){
        solverSymplecticEuler(m_particles.at(i), m_delta_t);
    }

    // gui stuff...
    m_asset_manager->m_planetary_system->updateRenderBuffers(centuries_since_j2000);
    m_planetarium_gui->setSelectedPlanet(m_bodies.at(m_pick)->getId());
    m_planetarium_gui->setSimulationDeltaT(m_delta_t);
    m_planetarium_gui->onFramebufferSizeUpdate(); // it's nasty to do this evey frame...
    m_physics->setCurrentTime(m_seconds_since_j2000);
    m_planetarium_gui->update();
    updatePredictionBuffer();

    m_seconds_since_j2000 += m_delta_t;
    render();
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

