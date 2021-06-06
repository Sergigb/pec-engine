#include <unordered_map>
#include <ctime>
#include <cmath>

#include "Planetarium.hpp"
#include "core/log.hpp"
#include "core/utils/utils.hpp"
#include "core/loading/load_star_system.hpp"
#include "core/maths_funcs.hpp"
#include "core/RenderContext.hpp"
#include "GUI/FontAtlas.hpp"


Planetarium::Planetarium() : BaseApp(){
    init();
}


Planetarium::Planetarium(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


void Planetarium::init(){
    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);
}


Planetarium::~Planetarium(){
}


void update_orbital_elements(planetary_system& system, const double centuries_since_j2000){
    planet_map::iterator it;
    planet_map& planets = system.planets;

    for(it=planets.begin();it!=planets.end();it++){
        planet& current = it->second;

        current.semi_major_axis = current.semi_major_axis_0 +
                                  current.semi_major_axis_d * centuries_since_j2000;
        current.eccentricity = current.eccentricity_0 +
                               current.eccentricity_d * centuries_since_j2000;
        current.inclination = current.inclination_0 +
                              current.inclination_d * centuries_since_j2000;
        current.mean_longitude = current.mean_longitude_0 +
                                 current.mean_longitude_d * centuries_since_j2000;
        current.longitude_perigee = current.longitude_perigee_0 +
                                    current.longitude_perigee_d * centuries_since_j2000;
        current.long_asc_node = current.long_asc_node_0 +
                                current.long_asc_node_d * centuries_since_j2000;

        current.mean_anomaly = current.mean_longitude - current.longitude_perigee;
        current.arg_periapsis = current.longitude_perigee - current.long_asc_node;

        current.eccentric_anomaly = current.mean_anomaly;
        double ecc_d = 10.8008135;
        while(std::abs(ecc_d) > 1e-6){
            ecc_d = (current.eccentric_anomaly - current.eccentricity *
                     std::sin(current.eccentric_anomaly) - current.mean_anomaly) / 
                     (1 - current.eccentricity * std::cos(current.eccentric_anomaly));
            current.eccentric_anomaly -= ecc_d;
        }

        current.pos_prev = current.pos;

        // results in a singularity as e -> 1
        double true_annomaly = 2 * std::atan(std::sqrt((1 + current.eccentricity)/
                                                       (1 - current.eccentricity)) * 
                                             std::tan(current.eccentric_anomaly / 2));

        double rad = current.semi_major_axis * (1 - current.eccentricity * 
                                                std::cos(current.eccentric_anomaly)) * AU_TO_METERS;

        current.pos.v[0] = rad * (std::cos(current.long_asc_node) * std::cos(current.arg_periapsis + true_annomaly) -
                           std::sin(current.long_asc_node) * std::sin(current.arg_periapsis + true_annomaly) *
                           std::cos(current.inclination));

        current.pos.v[1] = rad * (std::sin(current.long_asc_node) * std::cos(current.arg_periapsis + true_annomaly) +
                           std::cos(current.long_asc_node) * std::sin(current.arg_periapsis + true_annomaly) *
                           std::cos(current.inclination));

        current.pos.v[2] = rad * (std::sin(current.inclination) * std::sin(current.arg_periapsis + true_annomaly));
    }
}


/*

current.pos_prev = current.pos;

double P = AU_TO_METERS * current.semi_major_axis * (std::cos(current.eccentric_anomaly) -
                   current.eccentricity);
double Q = AU_TO_METERS * current.semi_major_axis * std::sin(current.eccentric_anomaly) *
           std::sqrt(1 - current.eccentricity * current.eccentricity);


current.pos.v[0] = std::cos(current.arg_periapsis) * P - 
                   std::sin(current.arg_periapsis) * Q;
current.pos.v[1] = std::sin(current.arg_periapsis) * P +
                   std::cos(current.arg_periapsis) * Q;

current.pos.v[2] = std::sin(current.inclination) * current.test.v[1];
current.pos.v[1] = std::cos(current.inclination) * current.test.v[1];

double xtemp = current.test.v[0];
current.pos.v[0] = std::cos(current.long_asc_node) * xtemp - 
                   std::sin(current.long_asc_node) * current.test.v[1];
current.pos.v[1] = std::sin(current.long_asc_node) * xtemp + 
                   std::cos(current.long_asc_node) * current.test.v[1];

*/


void Planetarium::run(){
    /*m_camera->setCameraPosition(dmath::vec3(9300000.0, 0.0, 0.0));
    m_camera->setSpeed(630000.0f);
    m_camera->createProjMat(1.0, 63000000, 67.0, 1.0);

    //glDisable(GL_CULL_FACE);

    m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));
    m_render_context->toggleDebugOverlay();

    while(!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
        m_player->update();

        //m_render_context->contextUpdatePlanetarium();

        if(m_input->pressed_keys[GLFW_KEY_R] & INPUT_KEY_RELEASE){
            m_render_context->reloadShaders();
            std::cout << "Shaders reloaded" << std::endl;
        }

        glfwSwapBuffers(m_window_handler->getWindow());
    }

    m_window_handler->terminate();*/
    double seconds_since_j2000 = 0.0, centuries_since_j2000 = 0.0;
    double delta_t = (1000. / 60.) / 1000.0;
    time_t current_time;
    const char* ctime_now;

    struct planetary_system system;
    load_star_system(system);

    while(1){
        centuries_since_j2000 = seconds_since_j2000 / SECONDS_IN_A_CENTURY;
        update_orbital_elements(system, centuries_since_j2000);

        current_time = (time_t)seconds_since_j2000 + SECS_FROM_UNIX_TO_J2000;
        ctime_now = ctime(&current_time);
        std::cout << ctime_now;

        std::hash<std::string> str_hash;
        planet& earth = system.planets.at(str_hash("Earth"));
        //dmath::vec3 disp = earth.pos - earth.pos_prev;
        std::cout << earth.pos.v[0] << ", " << earth.pos.v[1] << ", " << earth.pos.v[2] << std::endl;
        //double velocity = dmath::length(disp) / delta_t;

        //std::cout.precision(10);
        //std::cout << "vel earth: " << velocity << std::endl;

        seconds_since_j2000 += delta_t;
    }
}

