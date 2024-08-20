#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>
#include <stb/stb_image.h>

#include "PlanetariumRenderer.hpp"
#include "../game_components/GameSimulation.hpp"
#include "../core/BaseApp.hpp"
#include "../core/Physics.hpp"
#include "../core/AssetManager.hpp"
#include "../core/RenderContext.hpp"
#include "../core/Camera.hpp"
#include "../core/Player.hpp"
#include "../core/utils/gl_utils.hpp"
#include "../core/Predictor.hpp"
#include "../assets/Planet.hpp"
#include "../assets/PlanetarySystem.hpp"
#include "../assets/Vessel.hpp"
#include "../assets/BasePart.hpp"
#include "../GUI/planetarium/PlanetariumGUI.hpp"


PlanetariumRenderer::PlanetariumRenderer(BaseApp* app, PlanetariumGUI* planetarium_gui){
    m_app = app;

    m_render_context = m_app->getRenderContext();
    m_planetarium_gui = planetarium_gui;

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
    initBuffers();
}


PlanetariumRenderer::~PlanetariumRenderer(){
    glDeleteBuffers(1, &m_pred_vbo_vert);
    glDeleteBuffers(1, &m_pred_vbo_ind);
    glDeleteVertexArrays(1, &m_pred_vao);
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

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderSkybox(rbuf->view_mat);

    m_render_context->useProgram(SHADER_DEBUG);

    glUniformMatrix4fv(m_debug_view_mat, 1, GL_FALSE, rbuf->view_mat.m);
    glUniformMatrix4fv(m_debug_proj_mat, 1, GL_FALSE, m_app->getCamera()->getProjMatrix().m);

    renderOrbits(rbuf->planet_buffer);
    renderPredictions();

    check_gl_errors(true, "PlanetariumRenderer::render");
    return 0;
}


void PlanetariumRenderer::renderPredictions(){
    const Vessel* user_vessel = m_app->getPlayer()->getVessel();
    const Predictor* predictor = m_app->getPredictor();
    std::vector<struct particle_state> states;
    std::vector<std::vector<GLfloat>> prediction_buffers;
    struct fixed_time_trajectory_config config = m_planetarium_gui->getPredictorConfig();
    config.predictor_scale = PLANETARIUM_SCALE_FACTOR;
    config.predictor_start_time = m_app->getPhysics()->getCurrentTime();

    if(user_vessel == nullptr){
        return;
    }
    else{
        btVector3 bvec3;
        dmath::vec3 vessel_com, vessel_vel;

        bvec3 = user_vessel->getCoM();
        vessel_com = dmath::vec3(bvec3.getX(), bvec3.getY(), bvec3.getZ());

        bvec3 = user_vessel->getRoot()->m_body->getLinearVelocity();
        vessel_vel = dmath::vec3(bvec3.getX(), bvec3.getY(), bvec3.getZ());

        states.emplace_back(vessel_com, vessel_vel, user_vessel->getTotalMass());
    }
    
    std::unique_ptr<GLuint[]> index_buffer;
    index_buffer.reset(new GLuint[2 * config.predictor_steps]);

    // can't find a smarter way to initialise this buffer
    for(int i=0; i < config.predictor_steps; i++){
        index_buffer[i * 2] = i;
        index_buffer[i * 2 + 1] = i + 1;
    }

    predictor->computeTrajectoriesRender(prediction_buffers, states, config);

    m_render_context->bindVao(m_pred_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_pred_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, prediction_buffers.at(0).size() * sizeof(GLfloat),
                 &prediction_buffers.at(0)[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_pred_vbo_ind);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * config.predictor_steps * sizeof(GLuint),
                 index_buffer.get(), GL_STATIC_DRAW);

    m_render_context->useProgram(SHADER_DEBUG);

    glUniform3f(m_debug_color_location, 0.f, 1.f, 0.f);
    glUniform1f(m_debug_alpha_location, 1.f);

    glDrawElements(GL_LINES, config.predictor_steps * 2, GL_UNSIGNED_INT, NULL);

    check_gl_errors(true, "PlanetariumRenderer::renderPredictions");
}


void PlanetariumRenderer::renderOrbits(const std::vector<planet_transform>& buff){
    m_render_context->useProgram(SHADER_DEBUG);

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
    check_gl_errors(true, "PlanetariumRenderer::renderOrbits");
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
