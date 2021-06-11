#include <unordered_map>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <iomanip>

#include "Planetarium.hpp"
#include "core/log.hpp"
#include "core/utils/utils.hpp"
#include "core/maths_funcs.hpp"
#include "core/RenderContext.hpp"
#include "core/Camera.hpp"
#include "core/Input.hpp"
#include "core/WindowHandler.hpp"
#include "core/Player.hpp"
#include "core/Frustum.hpp"
#include "GUI/FontAtlas.hpp"
#include "GUI/Text2D.hpp"


Planetarium::Planetarium() : BaseApp(){
    init();
}


Planetarium::Planetarium(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


void Planetarium::init(){
    m_delta_t = (10000000000. / 60.0) / 1000.0;  // in ms
    load_star_system(m_system);
    initBuffers();
    update_orbital_elements(m_system, 0.0);

    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/VeraMoBI.ttf", 15);
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
        int iter = 0;
        while(std::abs(ecc_d) > 1e-6 && iter < MAX_ITER){
            ecc_d = (current.eccentric_anomaly - current.eccentricity *
                     std::sin(current.eccentric_anomaly) - current.mean_anomaly) / 
                     (1 - current.eccentricity * std::cos(current.eccentric_anomaly));
            current.eccentric_anomaly -= ecc_d;
            iter++;
        }

        current.pos_prev = current.pos;

        // results in a singularity as e -> 1
        double true_anomaly = 2 * std::atan(std::sqrt((1 + current.eccentricity)/
                                                       (1 - current.eccentricity)) * 
                                             std::tan(current.eccentric_anomaly / 2));

        double rad = current.semi_major_axis * (1 - current.eccentricity * 
                                                std::cos(current.eccentric_anomaly)) * AU_TO_METERS;

        current.pos.v[0] = rad * (std::cos(current.long_asc_node) * std::cos(current.arg_periapsis + true_anomaly) -
                           std::sin(current.long_asc_node) * std::sin(current.arg_periapsis + true_anomaly) *
                           std::cos(current.inclination));

        current.pos.v[1] = rad * (std::sin(current.inclination) * std::sin(current.arg_periapsis + true_anomaly));

        current.pos.v[2] = rad * (std::sin(current.long_asc_node) * std::cos(current.arg_periapsis + true_anomaly) +
                           std::cos(current.long_asc_node) * std::sin(current.arg_periapsis + true_anomaly) *
                           std::cos(current.inclination));
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

GLuint color_location, proj_location, view_location; // change me :)

void Planetarium::initBuffers(){
    planet_map::iterator it;
    planet_map& planets = m_system.planets;

    for(it=planets.begin();it!=planets.end();it++){
        planet& current = it->second;

        glGenVertexArrays(1, &current.m_vao);
        m_render_context->bindVao(current.m_vao);

        glGenBuffers(1, &current.m_vbo_vert);
        glBindBuffer(GL_ARRAY_BUFFER, current.m_vbo_vert);
        glVertexAttribPointer(0, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);

        glGenBuffers(1, &current.m_vbo_ind);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, current.m_vbo_ind);
        glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);

        color_location = m_render_context->getUniformLocation(SHADER_DEBUG, "line_color");
        proj_location = m_render_context->getUniformLocation(SHADER_DEBUG, "proj");
        view_location = m_render_context->getUniformLocation(SHADER_DEBUG, "view");
    }
}

#define NUM_VERTICES 300

void Planetarium::updateOrbitBuffers(double current_time){
    planet_map::iterator it;
    planet_map& planets = m_system.planets;
    std::unique_ptr<GLfloat[]> vertex_buffer;
    std::unique_ptr<GLushort[]> index_buffer;

    for(it=planets.begin();it!=planets.end();it++){
        planet& current = it->second;

        vertex_buffer.reset(new GLfloat[3 * NUM_VERTICES]);
        index_buffer.reset(new GLushort[2 * NUM_VERTICES]);

        m_render_context->bindVao(current.m_vao);

        double mean_anomaly_d = current.mean_longitude_d - current.longitude_perigee_d;
        double period = (4 * M_PI) / mean_anomaly_d;

        double eccentricity, long_asc_node, arg_periapsis, inclination, semi_major_axis,
               mean_longitude, longitude_perigee, mean_anomaly;
        double time = current_time;
        for(uint i=0; i < NUM_VERTICES; i++){
            time += period / NUM_VERTICES;

            semi_major_axis = current.semi_major_axis_0 + current.semi_major_axis_d * time;
            eccentricity = current.eccentricity_0 + current.eccentricity_d * time;
            inclination = current.inclination_0 + current.inclination_d * time;
            mean_longitude = current.mean_longitude_0 + current.mean_longitude_d * time;
            longitude_perigee = current.longitude_perigee_0 + current.longitude_perigee_d * time;
            long_asc_node = current.long_asc_node_0 + current.long_asc_node_d * time;

            mean_anomaly = mean_longitude - longitude_perigee;
            arg_periapsis = longitude_perigee - long_asc_node;

            double eccentric_anomaly = mean_anomaly;
            double ecc_d = 10.8008135;
            int iter = 0;
            while(std::abs(ecc_d) > 1e-6 && iter < MAX_ITER){
                ecc_d = (eccentric_anomaly - eccentricity *
                         std::sin(eccentric_anomaly) - mean_anomaly) / 
                         (1 - eccentricity * std::cos(eccentric_anomaly));
                eccentric_anomaly -= ecc_d;
            }

            double true_anomaly = 2 * std::atan(std::sqrt((1 + eccentricity)/
                                                           (1 - eccentricity)) * 
                                                 std::tan(eccentric_anomaly / 2));

            double rad = semi_major_axis * (1 - eccentricity * std::cos(eccentric_anomaly)) * (AU_TO_METERS / 1e10);

            vertex_buffer[i * 3] = rad * (std::cos(long_asc_node) * std::cos(arg_periapsis + true_anomaly) -
                                   std::sin(long_asc_node) * std::sin(arg_periapsis + true_anomaly) *
                                   std::cos(inclination));

            vertex_buffer[i * 3 + 1] = rad * (std::sin(inclination) * std::sin(arg_periapsis + true_anomaly));

            vertex_buffer[i * 3 + 2] = rad * (std::sin(long_asc_node) * std::cos(arg_periapsis + true_anomaly) +
                                       std::cos(long_asc_node) * std::sin(arg_periapsis + true_anomaly) *
                                       std::cos(inclination));

            index_buffer[i * 2] = i;
            index_buffer[i * 2 + 1] = i + 1;
        }
        index_buffer[2 * NUM_VERTICES - 1] = 0;

        glBindBuffer(GL_ARRAY_BUFFER, current.m_vbo_vert);
        glBufferData(GL_ARRAY_BUFFER, 3 * NUM_VERTICES * sizeof(GLfloat), vertex_buffer.get(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, current.m_vbo_ind);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * NUM_VERTICES * sizeof(GLushort), index_buffer.get(), GL_STATIC_DRAW);
    }
}


void Planetarium::renderOrbits(){
    planet_map::iterator it;
    planet_map& planets = m_system.planets;

    m_render_context->useProgram(SHADER_DEBUG);
    glUniformMatrix4fv(view_location, 1, GL_FALSE, m_camera->getViewMatrix().m);
    glUniformMatrix4fv(proj_location, 1, GL_FALSE, m_camera->getProjMatrix().m);
    glUniform3f(color_location, 1.0, 0.0, 0.0);

    for(it=planets.begin();it!=planets.end();it++){
        planet& current = it->second;

        m_render_context->bindVao(current.m_vao);

        glDrawElements(GL_LINES, NUM_VERTICES, GL_UNSIGNED_SHORT, NULL);
    }
}


void Planetarium::processInput(){
    double scx, scy;

    m_input->getScroll(scx, scy);

    if(scy < 0.0)
        m_delta_t *= 0.1f;
    if(scy > 0.0)
        m_delta_t *= 10.f;

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
}


void Planetarium::logic(){
    double centuries_since_j2000 = 0.0;
    time_t current_time;
    wchar_t buff[256];
    std::ostringstream oss;

    processInput();

    centuries_since_j2000 = m_seconds_since_j2000 / SECONDS_IN_A_CENTURY;
    update_orbital_elements(m_system, centuries_since_j2000);
    updateSceneText();

    m_text2->clearStrings();

    current_time = (time_t)m_seconds_since_j2000 + SECS_FROM_UNIX_TO_J2000;
    mbstowcs(buff, ctime(&current_time), 256);
    m_text2->addString(buff, 25, 50, 1.0f,
                      STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_RIGHT);

    oss << "delta_t: " << m_delta_t << "ms (" << std::setprecision(3) <<
            m_delta_t / 36000.0 << " hours)";
    mbstowcs(buff, oss.str().c_str(), 256);
    m_text2->addString(buff, 25, 35, 1.0f,
                      STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_RIGHT);

    m_seconds_since_j2000 += m_delta_t;

    updateOrbitBuffers(centuries_since_j2000);
    renderOrbits();

    m_text->render();
    m_text2->render();
}


void Planetarium::updateSceneText(){
    planet_map::iterator it;
    planet_map& planets = m_system.planets;
    const math::mat4 proj_mat = m_camera->getProjMatrix();
    const math::mat4 view_mat = m_camera->getViewMatrix();
    int w, h;

    m_window_handler->getFramebufferSize(w, h);

    m_text->clearStrings();

    for(it=planets.begin();it!=planets.end();it++){
        planet& current = it->second;
        wchar_t name[32];

        math::vec4 pos(current.pos.v[0] / 1e10, current.pos.v[1] / 1e10, current.pos.v[2] / 1e10, 1.0f);
        math::vec4 pos_screen = proj_mat * view_mat * pos;
        pos_screen = ((pos_screen / pos_screen.v[3]) + 1. ) / 2.; // there's something wrong here

        mbstowcs(name, current.name.c_str(), 32);
        m_text->addString(name, pos_screen.v[0] * w, pos_screen.v[1] * h, 1.0f,
                          STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);

    }
}


void Planetarium::run(){
    m_camera->setCameraPosition(dmath::vec3(500.0, 1000.0, 0.0));
    m_camera->setSpeed(50.0f);
    m_camera->createProjMat(1.0, 100000.0, 67.0, 1.0);

    //glDisable(GL_CULL_FACE);

    m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));

    while(!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
        m_player->update();

        m_render_context->contextUpdatePlanetRenderer(); // works in here too :)
        glClearColor(0.f, 0.f, 0.f, 0.f); // loook at the todo list
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        logic();

        glfwSwapBuffers(m_window_handler->getWindow());
    }

    m_window_handler->terminate();
}

