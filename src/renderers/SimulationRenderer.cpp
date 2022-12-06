#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "SimulationRenderer.hpp"
#include "../core/BaseApp.hpp"
#include "../core/Physics.hpp"
#include "../core/RenderContext.hpp"
#include "../core/Camera.hpp"
#include "../assets/Planet.hpp"
#include "../assets/Object.hpp"


SimulationRenderer::SimulationRenderer(BaseApp* app){
    m_app = app;

    m_render_context = m_app->getRenderContext();
    m_camera = m_app->getCamera();

    m_render_context->useProgram(SHADER_PHONG_BLINN_NO_TEXTURE);
    m_pb_notex_view_mat = m_render_context->getUniformLocation(SHADER_PHONG_BLINN_NO_TEXTURE,
                                                             "view");
    m_pb_notex_proj_mat = m_render_context->getUniformLocation(SHADER_PHONG_BLINN_NO_TEXTURE,
                                                             "proj");
    m_render_context->useProgram(SHADER_PHONG_BLINN);
    m_pb_view_mat = m_render_context->getUniformLocation(SHADER_PHONG_BLINN, "view");
    m_pb_proj_mat = m_render_context->getUniformLocation(SHADER_PHONG_BLINN, "proj");
    m_render_context->useProgram(SHADER_PLANET);
    m_planet_view_mat = m_render_context->getUniformLocation(SHADER_PLANET, "view");
    m_planet_proj_mat = m_render_context->getUniformLocation(SHADER_PLANET, "proj");
}


SimulationRenderer::~SimulationRenderer(){

}


int SimulationRenderer::render(struct render_buffer* rbuf){
    int num_rendered = 0;

    num_rendered = renderObjects(rbuf->buffer, rbuf->view_mat);

    for(uint i=0; i < rbuf->planet_buffer.size(); i++){
        planet_transform& tr = rbuf->planet_buffer.at(i);
        tr.planet_ptr->render(rbuf->cam_origin, tr.transform);
    }

    rbuf->buffer_lock.unlock();

    return num_rendered;
}


int SimulationRenderer::renderObjects(const std::vector<object_transform>& buff, 
                                      const math::mat4& view_mat){
    int num_rendered = 0;

    m_render_context->useProgram(SHADER_PHONG_BLINN_NO_TEXTURE);
    glUniformMatrix4fv(m_pb_notex_view_mat, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(m_pb_notex_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    m_render_context->useProgram(SHADER_PHONG_BLINN);
    glUniformMatrix4fv(m_pb_view_mat, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(m_pb_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    m_render_context->useProgram(SHADER_PLANET);
    glUniformMatrix4fv(m_planet_view_mat, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(m_planet_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    for(uint i=0; i<buff.size(); i++){
        num_rendered += buff.at(i).object_ptr->render(buff.at(i).transform);
    }

    return num_rendered;
}
