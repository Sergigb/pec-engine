#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "PlanetariumRenderer.hpp"
#include "../core/BaseApp.hpp"
#include "../core/Physics.hpp"
#include "../core/AssetManager.hpp"
#include "../core/RenderContext.hpp"
#include "../core/Camera.hpp"
#include "../core/Player.hpp"
#include "../assets/Planet.hpp"
#include "../assets/PlanetarySystem.hpp"


PlanetariumRenderer::PlanetariumRenderer(BaseApp* app){
    m_app = app;

    RenderContext* render_context = m_app->getRenderContext();

    render_context->useProgram(SHADER_DEBUG);
    m_debug_view_mat = render_context->getUniformLocation(SHADER_DEBUG, "view");
    m_debug_proj_mat = render_context->getUniformLocation(SHADER_DEBUG, "proj");
    m_debug_color_location = render_context->getUniformLocation(SHADER_DEBUG, "line_color");
}


PlanetariumRenderer::~PlanetariumRenderer(){

}


int PlanetariumRenderer::render(struct render_buffer* rbuf){
    m_app->getAssetManager()->m_planetary_system->updateRenderBuffers(
        m_app->getPhysics()->getCurrentTime() / SECONDS_IN_A_CENTURY);
    renderPlanetariumOrbits(rbuf->planet_buffer, rbuf->view_mat);

    return 0;
}


void PlanetariumRenderer::renderPlanetariumOrbits(const std::vector<planet_transform>& buff, const math::mat4& view_mat){
    RenderContext* render_context = m_app->getRenderContext();

    render_context->useProgram(SHADER_DEBUG);

    glUniformMatrix4fv(m_debug_view_mat, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(m_debug_proj_mat, 1, GL_FALSE, m_app->getCamera()->getProjMatrix().m);

    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for(uint i=0; i < buff.size(); i++){
        Planet* current = buff.at(i).planet_ptr;

        if(current->getId() == m_app->getPlayer()->getPlanetariumSelectedPlanet()){
            glUniform3f(m_debug_color_location, 0.0, 1.0, 0.0);
        }
        else{
            glUniform3f(m_debug_color_location, 1.0, 0.0, 0.0);
        }
        current->renderOrbit();
    }
}
