#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>
#include <stb/stb_image.h>

#include "PlanetariumRenderer.hpp"
#include "../core/BaseApp.hpp"
#include "../core/Physics.hpp"
#include "../core/AssetManager.hpp"
#include "../core/RenderContext.hpp"
#include "../core/Camera.hpp"
#include "../core/Player.hpp"
#include "../core/utils/gl_utils.hpp"
#include "../assets/Planet.hpp"
#include "../assets/PlanetarySystem.hpp"
#include "../assets/Vessel.hpp"


PlanetariumRenderer::PlanetariumRenderer(BaseApp* app){
    m_app = app;

    m_render_context = m_app->getRenderContext();

    m_render_context->useProgram(SHADER_DEBUG);
    m_debug_view_mat = m_render_context->getUniformLocation(SHADER_DEBUG, "view");
    m_debug_proj_mat = m_render_context->getUniformLocation(SHADER_DEBUG, "proj");
    m_debug_color_location = m_render_context->getUniformLocation(SHADER_DEBUG, "line_color");
    m_debug_alpha_location = m_render_context->getUniformLocation(SHADER_DEBUG, "alpha");

    m_skybox_view_loc = m_render_context->getUniformLocation(SHADER_TEXTURE_NO_LIGHT, "view");
    m_skybox_proj_loc = m_render_context->getUniformLocation(SHADER_TEXTURE_NO_LIGHT, "proj");
    m_skybox_model_loc = m_render_context->getUniformLocation(SHADER_TEXTURE_NO_LIGHT, "model");

    m_target_fade = 0.0;

    createSkybox();
}


PlanetariumRenderer::~PlanetariumRenderer(){

}


void PlanetariumRenderer::initBuffers(){
    glGenVertexArrays(1, &m_pred_vao);
    m_render_context->bindVao(m_pred_vao);

    glGenBuffers(1, &m_pred_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_pred_vbo_vert);
    glVertexAttribPointer(0, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_pred_vbo_ind);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pred_vbo_ind);
}


int PlanetariumRenderer::render(struct render_buffer* rbuf){
    m_app->getAssetManager()->m_planetary_system->updateRenderBuffers(
        m_app->getPhysics()->getCurrentTime() / SECONDS_IN_A_CENTURY);
    renderOrbits(rbuf->planet_buffer, rbuf->view_mat);
    renderPredictions(rbuf->view_mat);

    return 0;
}

// declare the following in the header
uint m_predictor_steps = 200;
uint m_predictor_period = 356; // in days?

void PlanetariumRenderer::renderPredictions(const math::mat4& view_mat){
    planet_map::const_iterator it;
    const planet_map& planets = m_app->getAssetManager()->m_planetary_system->getPlanets();
    double star_mass = m_app->getAssetManager()->m_planetary_system->getStar().mass;
    double vessel_mass;
    const Vessel* user_vessel = m_app->getPlayer()->getVessel();
    btVector3 vessel_com_bullet;
    dmath::vec3 vessel_com;

    if(user_vessel == nullptr){
        return;
    }
    else{
        vessel_com_bullet = user_vessel->getCoM();
        vessel_com = dmath::vec3(vessel_com_bullet.getX(),
                                 vessel_com_bullet.getY(),
                                 vessel_com_bullet.getZ());
        vessel_mass = user_vessel->getTotalMass();
    }

    std::unique_ptr<GLfloat[]> vertex_buffer;
    std::unique_ptr<GLuint[]> index_buffer;

    vertex_buffer.reset(new GLfloat[3 * m_predictor_steps]);
    index_buffer.reset(new GLuint[2 * m_predictor_steps]);

    m_render_context->bindVao(m_pred_vao);

    double elapsed_time = m_app->getPhysics()->getCurrentTime();

    double e, W, w, inc, a, L, p, M, v;
    double time = elapsed_time / SECONDS_IN_A_CENTURY;
    double predictor_delta_t_secs = (m_predictor_period * 24 * 60 * 60) / m_predictor_steps;
    double predictor_delta_t_cent = predictor_delta_t_secs / SECONDS_IN_A_CENTURY;

    for(uint i=0; i < m_predictor_steps; i++){
        dmath::vec3 force_on_vessel;  // in nm
        for(it=planets.begin();it!=planets.end();it++){
            const orbital_data& data = it->second->getOrbitalData();
            dmath::vec3 planet_origin;

            // update planet position
            a = data.a_0 + data.a_d * time;
            e = data.e_0 + data.e_d * time;
            inc = data.i_0 + data.i_d * time;
            L = data.L_0 + data.L_d * time;
            p = data.p_0 + data.p_d * time;
            W = data.W_0 + data.W_d * time;


            M = L - p;
            w = p - W;

            double E = M;
            double ecc_d = 10.;
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

            // update gravity force on current object
            double Rh = dmath::distance(planet_origin, vessel_com);

            double acceleration = GRAVITATIONAL_CONSTANT * (data.m / (Rh*Rh));
            dmath::vec3 f = dmath::normalise(planet_origin - vessel_com)
                                             * acceleration * vessel_mass;
            force_on_vessel += f;
        }

        // force of the star
        double Rh = dmath::length(vessel_com); // centered star com at (0, 0, 0)
        double acceleration = GRAVITATIONAL_CONSTANT * (star_mass / (Rh*Rh));
        dmath::vec3 f = dmath::normalise(dmath::vec3(-vessel_com.v[0],
                                                     -vessel_com.v[1],
                                                     -vessel_com.v[2])) * acceleration * vessel_mass;
        force_on_vessel += f;


        // udpate buffers
        time += predictor_delta_t_cent;

        vertex_buffer[i * 3] = vessel_com.v[0] / 1e10;
        vertex_buffer[i * 3 + 1] = vessel_com.v[1] / 1e10;
        vertex_buffer[i * 3 + 2] = vessel_com.v[2] / 1e10;

        index_buffer[i * 2] = i;
        index_buffer[i * 2 + 1] = i + 1;
    }

}


inline void solverSymplecticEuler(dmath::vec3 force, double delta_t){
    dmath::vec3 acceleration = force / p.mass;

    dmath::vec3 velocity_tp1 = p.velocity + acceleration * delta_t;
    p.origin = p.origin + velocity_tp1 * delta_t;

    p.velocity = velocity_tp1;
    p.total_force.v[0] = 0.0;
    p.total_force.v[1] = 0.0;
    p.total_force.v[2] = 0.0;
}


void PlanetariumRenderer::renderOrbits(const std::vector<planet_transform>& buff,
                                                  const math::mat4& view_mat){
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSkybox(view_mat);

    m_render_context->useProgram(SHADER_DEBUG);

    glUniformMatrix4fv(m_debug_view_mat, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(m_debug_proj_mat, 1, GL_FALSE, m_app->getCamera()->getProjMatrix().m);

    for(uint i=0; i < buff.size(); i++){
        const Planet* current = buff.at(i).planet_ptr;
        const math::vec3& color = current->getBaseColor();

        if(current->getId() == m_app->getPlayer()->getPlanetariumSelectedPlanet()){
            glUniform3f(m_debug_color_location, color.v[0], color.v[1], color.v[2]);
            glUniform1f(m_debug_alpha_location, 1 - m_target_fade);
        }
        else{
            glUniform3f(m_debug_color_location, color.v[0]*0.5, color.v[1]*0.5, color.v[2]*0.5);
            glUniform1f(m_debug_alpha_location, 1 - m_target_fade);
        }
        current->renderOrbit();
    }
}


void PlanetariumRenderer::createSkybox(){
    glGenVertexArrays(1, &m_vao);
    m_render_context->bindVao(m_vao);

    GLfloat vertices[] = {-0.5f, 0.5f, .0f, 
                          -0.5f, -0.5f, .0f,
                          0.5f, 0.5f, .0f,
                          0.5f, 0.5f, .0f,
                          -0.5f, -0.5f, .0f,
                          0.5f, -0.5f, .0f};

    GLfloat tex_coords[] = {0.0f, 0.0f,
                            0.0f, 1.0f,
                            1.0f, 0.0f,
                            1.0f, 0.0f,
                            0.0f, 1.0f,
                            1.0f, 1.0f,};

    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, 3 * 6 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glBufferData(GL_ARRAY_BUFFER, 2 * 6 * sizeof(GLfloat), tex_coords, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);

    check_gl_errors(true, "createSkybox");

    for(uint i=0; i < 6; i++){
        m_skybox_transforms[i] = math::identity_mat4();
        m_skybox_transforms[i] = math::scale(m_skybox_transforms[i],
                                             math::vec3(SKYBOX_SIZE, SKYBOX_SIZE, SKYBOX_SIZE));
        if(i < 4){
            m_skybox_transforms[i] = math::rotate_y_deg(m_skybox_transforms[i], -90.0f * i);
            if(i == 0)
                m_skybox_transforms[i] = math::translate(m_skybox_transforms[i],
                                                         math::vec3(0.f, 0.f, -SKYBOX_SIZE / 2.f));
            else if(i == 1)
                m_skybox_transforms[i] = math::translate(m_skybox_transforms[i],
                                                         math::vec3(SKYBOX_SIZE / 2.f,
                                                                    0.0f, 0.f));
            else if(i == 2)
                m_skybox_transforms[i] = math::translate(m_skybox_transforms[i],
                                                         math::vec3(0.f, 0.f, SKYBOX_SIZE / 2));
            else if(i == 3)
                m_skybox_transforms[i] = math::translate(m_skybox_transforms[i],
                                                         math::vec3(-SKYBOX_SIZE / 2, 0.f, 0.f));
        }
        else{
            m_skybox_transforms[i] = math::rotate_x_deg(m_skybox_transforms[i], i == 4 ? 90 : -90);
            m_skybox_transforms[i] = math::translate(m_skybox_transforms[i],
                                                     i == 4 ? 
                                                     math::vec3(0.f, SKYBOX_SIZE / 2, 0.f) :
                                                     math::vec3(0.f, -SKYBOX_SIZE / 2, 0.f));
        }
    }

    for(uint i=0; i < 6; i++){
        std::ostringstream path;
        path << "../data/skybox/" << std::to_string(i) << ".png";

        int x, y, n;
        unsigned char* image_data = stbi_load(path.str().c_str(), &x, &y, &n, 3);
        if(!image_data) {
            std::cerr << "PlanetariumRenderer::createSkybox: failed to load skybox texture"
                         " with path " << path.str() << std::endl;
            log("PlanetariumRenderer::createSkybox: failed to load skybox texture with path ",
                path.str());
        }
        else{
            glGenTextures(1, &m_textures[i]);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_textures[i]);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, image_data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        }
        stbi_image_free(image_data);
        check_gl_errors(true, "createSkybox texture creation");
    }
}


void PlanetariumRenderer::renderSkybox(const math::mat4& view_mat){
    m_render_context->useProgram(SHADER_TEXTURE_NO_LIGHT);
    m_render_context->bindVao(m_vao);

    glUniformMatrix4fv(m_skybox_view_loc, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(m_skybox_proj_loc, 1, GL_FALSE, m_app->getCamera()->getProjMatrix().m);

    for(uint i=0; i < 6; i++){
        glUniformMatrix4fv(m_skybox_model_loc, 1, GL_FALSE, m_skybox_transforms[i].m);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_textures[i]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    check_gl_errors(true, "renderSkybox");
}


void PlanetariumRenderer::setTargetFade(float value){
    m_target_fade = value;
}
