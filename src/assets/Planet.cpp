#include <stb/stb_image.h>
#include <thread>

#include "Planet.hpp"
#include "../core/common.hpp"
#include "../core/RenderContext.hpp"
#include "../core/log.hpp"
#include "../core/Physics.hpp"


#define TEXTURE_LOCATION 0
#define ELEVATION_LOCATION 1


Planet::Planet(RenderContext* render_context) : m_planet_tree(render_context, this){
    m_render_context = render_context;
    initBuffers();
}


Planet::~Planet(){
    glDeleteBuffers(1, &m_vbo_vert);
    glDeleteBuffers(1, &m_vbo_ind);
    glDeleteVertexArrays(1, &m_vao);
}


const dmath::mat4 Planet::getTransform() const{
    dmath::mat4 t;
    dmath::translate(t, m_orbital_data.pos);
    return t;
}


const orbital_data& Planet::getOrbitalData() const{
    return m_orbital_data;
}


const dmath::vec3& Planet::getPosition() const{
    return m_orbital_data.pos;
}


const dmath::vec3& Planet::getPrevPosition() const{
    return m_orbital_data.pos_prev;
}


const std::string& Planet::getName() const{
    return m_name;
}


std::uint32_t Planet::getId() const{
    return m_id;
}


orbital_data& Planet::getOrbitalData(){
    return m_orbital_data;
}


void Planet::setID(std::uint32_t id){
    m_id = id;
}


void Planet::setName(const char* name){
    m_name = name;
}


void Planet::render(const dmath::vec3& cam_translation){
    log("warning: uninplemented mathod Planet::render(const dmath::vec3& cam_translation)");
    std::cerr << "warning: uninplemented mathod Planet::render(const dmath::vec3& cam_translation)" << std::endl;
    m_planet_tree.render(cam_translation, dmath::identity_mat4());
}

void Planet::render(const dmath::vec3& cam_translation, const dmath::mat4 transform){
    m_planet_tree.render(cam_translation, transform);
}


void Planet::buildSurface(){
    m_planet_tree.buildSurface();
}


void Planet::destroySurface(){
    // meh
}


void Planet::initBuffers(){
    glGenVertexArrays(1, &m_vao);
    m_render_context->bindVao(m_vao);

    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glVertexAttribPointer(0, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_vbo_ind);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glVertexAttribPointer(1, 2, GL_UNSIGNED_SHORT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
}


void Planet::updateOrbitalElements(const double cent_since_j2000){
    m_orbital_data.semi_major_axis = m_orbital_data.semi_major_axis_0 +
                                     m_orbital_data.semi_major_axis_d * cent_since_j2000;
    m_orbital_data.eccentricity = m_orbital_data.eccentricity_0 +
                                  m_orbital_data.eccentricity_d * cent_since_j2000;
    m_orbital_data.inclination = m_orbital_data.inclination_0 +
                                 m_orbital_data.inclination_d * cent_since_j2000;
    m_orbital_data.mean_longitude = m_orbital_data.mean_longitude_0 +
                                    m_orbital_data.mean_longitude_d * cent_since_j2000;
    m_orbital_data.longitude_perigee = m_orbital_data.longitude_perigee_0 +
                                       m_orbital_data.longitude_perigee_d * cent_since_j2000;
    m_orbital_data.long_asc_node = m_orbital_data.long_asc_node_0 +
                                   m_orbital_data.long_asc_node_d * cent_since_j2000;

    m_orbital_data.mean_anomaly = m_orbital_data.mean_longitude - m_orbital_data.longitude_perigee;
    m_orbital_data.arg_periapsis = m_orbital_data.longitude_perigee - m_orbital_data.long_asc_node;

    m_orbital_data.eccentric_anomaly = m_orbital_data.mean_anomaly;
    double ecc_d = 10.8008135;
    int iter = 0;
    while(std::abs(ecc_d) > 1e-6 && iter < MAX_SOLVER_ITER){
        ecc_d = (m_orbital_data.eccentric_anomaly - m_orbital_data.eccentricity *
                 std::sin(m_orbital_data.eccentric_anomaly) - m_orbital_data.mean_anomaly) / 
                 (1 - m_orbital_data.eccentricity * std::cos(m_orbital_data.eccentric_anomaly));
        m_orbital_data.eccentric_anomaly -= ecc_d;
        iter++;
    }

    m_orbital_data.pos_prev = m_orbital_data.pos;

    // results in a singularity as e -> 1
    double true_anomaly = 2 * std::atan(std::sqrt((1 + m_orbital_data.eccentricity)/
                                                   (1 - m_orbital_data.eccentricity)) * 
                                        std::tan(m_orbital_data.eccentric_anomaly / 2));

    double rad = m_orbital_data.semi_major_axis * (1 - m_orbital_data.eccentricity * 
                                            std::cos(m_orbital_data.eccentric_anomaly)) * AU_TO_METERS;

    m_orbital_data.pos.v[0] = rad * (std::cos(m_orbital_data.long_asc_node) * std::cos(m_orbital_data.arg_periapsis +
                              true_anomaly) - std::sin(m_orbital_data.long_asc_node) * std::sin(
                              m_orbital_data.arg_periapsis + true_anomaly) * std::cos(m_orbital_data.inclination));

    m_orbital_data.pos.v[1] = rad * (std::sin(m_orbital_data.inclination) * std::sin(m_orbital_data.arg_periapsis+
                              true_anomaly));

    m_orbital_data.pos.v[2] = rad * (std::sin(m_orbital_data.long_asc_node) * std::cos(m_orbital_data.arg_periapsis +
                              true_anomaly) +std::cos(m_orbital_data.long_asc_node) * std::sin(
                              m_orbital_data.arg_periapsis + true_anomaly) * std::cos(m_orbital_data.inclination));
}


/*

alternative computation method, maybe we should benchmark it :^)

m_orbital_data.pos_prev = m_orbital_data.pos;

double P = AU_TO_METERS * m_orbital_data.semi_major_axis * (std::cos(m_orbital_data.eccentric_anomaly) -
                   m_orbital_data.eccentricity);
double Q = AU_TO_METERS * m_orbital_data.semi_major_axis * std::sin(m_orbital_data.eccentric_anomaly) *
           std::sqrt(1 - m_orbital_data.eccentricity * m_orbital_data.eccentricity);


m_orbital_data.pos.v[0] = std::cos(m_orbital_data.arg_periapsis) * P - 
                   std::sin(m_orbital_data.arg_periapsis) * Q;
m_orbital_data.pos.v[1] = std::sin(m_orbital_data.arg_periapsis) * P +
                   std::cos(m_orbital_data.arg_periapsis) * Q;

m_orbital_data.pos.v[2] = std::sin(m_orbital_data.inclination) * m_orbital_data.test.v[1];
m_orbital_data.pos.v[1] = std::cos(m_orbital_data.inclination) * m_orbital_data.test.v[1];

double xtemp = m_orbital_data.test.v[0];
m_orbital_data.pos.v[0] = std::cos(m_orbital_data.long_asc_node) * xtemp - 
                   std::sin(m_orbital_data.long_asc_node) * m_orbital_data.test.v[1];
m_orbital_data.pos.v[1] = std::sin(m_orbital_data.long_asc_node) * xtemp + 
                   std::cos(m_orbital_data.long_asc_node) * m_orbital_data.test.v[1];

*/


void Planet::updateRenderBuffers(double current_time){
    std::unique_ptr<GLfloat[]> vertex_buffer;
    std::unique_ptr<GLushort[]> index_buffer;

    vertex_buffer.reset(new GLfloat[3 * NUM_VERTICES]);
    index_buffer.reset(new GLushort[2 * NUM_VERTICES]);

    m_render_context->bindVao(m_vao);

    double eccentricity, long_asc_node, arg_periapsis, inclination, semi_major_axis,
           mean_longitude, longitude_perigee, mean_anomaly;
    double time = current_time;
    for(uint i=0; i < NUM_VERTICES; i++){
        time += m_orbital_data.period / NUM_VERTICES;

        semi_major_axis = m_orbital_data.semi_major_axis_0 + m_orbital_data.semi_major_axis_d * time;
        eccentricity = m_orbital_data.eccentricity_0 + m_orbital_data.eccentricity_d * time;
        inclination = m_orbital_data.inclination_0 + m_orbital_data.inclination_d * time;
        mean_longitude = m_orbital_data.mean_longitude_0 + m_orbital_data.mean_longitude_d * time;
        longitude_perigee = m_orbital_data.longitude_perigee_0 + m_orbital_data.longitude_perigee_d * time;
        long_asc_node = m_orbital_data.long_asc_node_0 + m_orbital_data.long_asc_node_d * time;

        mean_anomaly = mean_longitude - longitude_perigee;
        arg_periapsis = longitude_perigee - long_asc_node;

        double eccentric_anomaly = mean_anomaly;
        double ecc_d = 10.8008135;
        int iter = 0;
        while(std::abs(ecc_d) > 1e-6 && iter < MAX_SOLVER_ITER){
            ecc_d = (eccentric_anomaly - eccentricity *
                     std::sin(eccentric_anomaly) - mean_anomaly) / 
                     (1 - eccentricity * std::cos(eccentric_anomaly));
            eccentric_anomaly -= ecc_d;
        }

        m_orbital_data.true_anomaly = 2 * std::atan(std::sqrt((1 + eccentricity)/
                                                       (1 - eccentricity)) * 
                                             std::tan(eccentric_anomaly / 2));

        double rad = semi_major_axis * (1 - eccentricity * std::cos(eccentric_anomaly)) * (AU_TO_METERS / 1e10);

        vertex_buffer[i * 3] = rad * (std::cos(long_asc_node) * std::cos(arg_periapsis + m_orbital_data.true_anomaly) -
                               std::sin(long_asc_node) * std::sin(arg_periapsis + m_orbital_data.true_anomaly) *
                               std::cos(inclination));

        vertex_buffer[i * 3 + 1] = rad * (std::sin(inclination) * std::sin(arg_periapsis + m_orbital_data.true_anomaly));

        vertex_buffer[i * 3 + 2] = rad * (std::sin(long_asc_node) * std::cos(arg_periapsis + m_orbital_data.true_anomaly) +
                                   std::cos(long_asc_node) * std::sin(arg_periapsis + m_orbital_data.true_anomaly) *
                                   std::cos(inclination));

        index_buffer[i * 2] = i;
        index_buffer[i * 2 + 1] = i + 1;
    }
    index_buffer[(2 * NUM_VERTICES) - 1] = 0;

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 3 * NUM_VERTICES * sizeof(GLfloat), vertex_buffer.get(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * NUM_VERTICES * sizeof(GLushort), index_buffer.get(), GL_STATIC_DRAW);
}


void Planet::renderOrbit() const{
    m_render_context->bindVao(m_vao);
    glDrawElements(GL_LINES, NUM_VERTICES * 2, GL_UNSIGNED_SHORT, NULL);
}
