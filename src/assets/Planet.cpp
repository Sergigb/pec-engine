#include <stb/stb_image.h>
#include <thread>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "Planet.hpp"
#include "Kinematic.hpp"
#include "../core/common.hpp"
#include "../core/RenderContext.hpp"
#include "../core/log.hpp"
#include "../core/Physics.hpp"
#include "../GUI/Sprite.hpp"


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
    m_orbital_data.a = m_orbital_data.a_0 + m_orbital_data.a_d * cent_since_j2000;
    m_orbital_data.e = m_orbital_data.e_0 + m_orbital_data.e_d * cent_since_j2000;
    m_orbital_data.i = m_orbital_data.i_0 + m_orbital_data.i_d * cent_since_j2000;
    m_orbital_data.L = m_orbital_data.L_0 + m_orbital_data.L_d * cent_since_j2000;
    m_orbital_data.p = m_orbital_data.p_0 + m_orbital_data.p_d * cent_since_j2000;
    m_orbital_data.W = m_orbital_data.W_0 + m_orbital_data.W_d * cent_since_j2000;

    m_orbital_data.M = m_orbital_data.L - m_orbital_data.p;
    m_orbital_data.w = m_orbital_data.p - m_orbital_data.W;

    m_orbital_data.E = m_orbital_data.M;
    double ecc_d = 10.8008135;
    int iter = 0;
    while(std::abs(ecc_d) > 1e-6 && iter < MAX_SOLVER_ITER){
        ecc_d = (m_orbital_data.E - m_orbital_data.e * std::sin(m_orbital_data.E) - 
                 m_orbital_data.M) / (1 - m_orbital_data.e * std::cos(m_orbital_data.E));
        m_orbital_data.E -= ecc_d;
        iter++;
    }

    m_orbital_data.v = 2 * std::atan(std::sqrt((1 + m_orbital_data.e) / (1 - m_orbital_data.e))
                       * std::tan(m_orbital_data.E / 2));

    m_orbital_data.pos_prev = m_orbital_data.pos;

    // results in a singularity as e -> 1
    double v = 2 * std::atan(std::sqrt((1 + m_orbital_data.e) / (1 - m_orbital_data.e))
                                        * std::tan(m_orbital_data.E / 2));

    double rad = m_orbital_data.a * (1 - m_orbital_data.e * std::cos(m_orbital_data.E))
                                     * AU_TO_METERS;

    m_orbital_data.pos.v[0] = rad * (std::cos(m_orbital_data.W) * std::cos(m_orbital_data.w + v)
                                     - std::sin(m_orbital_data.W) * std::sin(m_orbital_data.w + v)
                                     * std::cos(m_orbital_data.i));
    m_orbital_data.pos.v[1] = rad * (std::sin(m_orbital_data.i) * std::sin(m_orbital_data.w + v));
    m_orbital_data.pos.v[2] = rad * (std::sin(m_orbital_data.W) * std::cos(m_orbital_data.w + v) +
                                     std::cos(m_orbital_data.W) * std::sin(m_orbital_data.w + v)
                                     * std::cos(m_orbital_data.i));

}


/*

alternative computation method, maybe we should benchmark it :^)

m_orbital_data.pos_prev = m_orbital_data.pos;

double P = AU_TO_METERS * m_orbital_data.a * (std::cos(m_orbital_data.E) -
                   m_orbital_data.e);
double Q = AU_TO_METERS * m_orbital_data.a * std::sin(m_orbital_data.E) *
           std::sqrt(1 - m_orbital_data.e * m_orbital_data.e);


m_orbital_data.pos.v[0] = std::cos(m_orbital_data.w) * P - 
                   std::sin(m_orbital_data.w) * Q;
m_orbital_data.pos.v[1] = std::sin(m_orbital_data.w) * P +
                   std::cos(m_orbital_data.w) * Q;

m_orbital_data.pos.v[2] = std::sin(m_orbital_data.i) * m_orbital_data.test.v[1];
m_orbital_data.pos.v[1] = std::cos(m_orbital_data.i) * m_orbital_data.test.v[1];

double xtemp = m_orbital_data.test.v[0];
m_orbital_data.pos.v[0] = std::cos(m_orbital_data.W) * xtemp - 
                   std::sin(m_orbital_data.W) * m_orbital_data.test.v[1];
m_orbital_data.pos.v[1] = std::sin(m_orbital_data.W) * xtemp + 
                   std::cos(m_orbital_data.W) * m_orbital_data.test.v[1];

*/


void Planet::updateRenderBuffers(double current_time){
    std::unique_ptr<GLfloat[]> vertex_buffer;
    std::unique_ptr<GLushort[]> index_buffer;

    vertex_buffer.reset(new GLfloat[3 * NUM_VERTICES]);
    index_buffer.reset(new GLushort[2 * NUM_VERTICES]);

    m_render_context->bindVao(m_vao);

    double e, W, w, inc, a, L, p, M, v;
    double time = current_time;
    for(uint i=0; i < NUM_VERTICES; i++){
        time += m_orbital_data.period / NUM_VERTICES;

        a = m_orbital_data.a_0 + m_orbital_data.a_d * time;
        e = m_orbital_data.e_0 + m_orbital_data.e_d * time;
        inc = m_orbital_data.i_0 + m_orbital_data.i_d * time;
        L = m_orbital_data.L_0 + m_orbital_data.L_d * time;
        p = m_orbital_data.p_0 + m_orbital_data.p_d * time;
        W = m_orbital_data.W_0 + m_orbital_data.W_d * time;

        M = L - p;
        w = p - W;

        double E = M;
        double ecc_d = 10.8008135;
        int iter = 0;
        while(std::abs(ecc_d) > 1e-6 && iter < MAX_SOLVER_ITER){
            ecc_d = (E - e * std::sin(E) - M) / (1 - e * std::cos(E));
            E -= ecc_d;
        }

        v = 2 * std::atan(std::sqrt((1 + e) / (1 - e)) * std::tan(E / 2));

        double rad = a * (1 - e * std::cos(E)) * (AU_TO_METERS / 1e10);

        vertex_buffer[i * 3] = rad * (std::cos(W) * std::cos(w + v) -
                               std::sin(W) * std::sin(w + v) * std::cos(inc));
        vertex_buffer[i * 3 + 1] = rad * (std::sin(inc) * std::sin(w + v));
        vertex_buffer[i * 3 + 2] = rad * (std::sin(W) * std::cos(w + v) +
                                   std::cos(W) * std::sin(w + v) *std::cos(inc));

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


void Planet::registerKinematic(Kinematic* kinematic){
    m_kinematics.emplace_back(kinematic);
}


void Planet::updateKinematics(){
    btVector3 origin(m_orbital_data.pos.v[0], m_orbital_data.pos.v[1], m_orbital_data.pos.v[2]);
    for(uint i=0; i < m_kinematics.size(); i++){
        m_kinematics.at(i)->update(origin, btQuaternion::getIdentity());
    }
}

void Planet::loadThumbnail(const char* path){
    m_planet_thumbnail.reset(new Sprite(m_render_context, math::vec2(.0f, .0f), SPRITE_DRAW_ABSOLUTE,
                                        path, 24.0f));
}
