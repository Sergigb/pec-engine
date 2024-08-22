#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "EditorRenderer.hpp"
#include "../core/BaseApp.hpp"
#include "../core/Physics.hpp"
#include "../core/RenderContext.hpp"
#include "../core/Camera.hpp"
#include "../core/utils/gl_utils.hpp"
#include "../assets/Object.hpp"
#include "../assets/Model.hpp"
#include "../assets/BasePart.hpp"


EditorRenderer::EditorRenderer(BaseApp* app){
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

    m_att_point_model.reset(new Model("../data/sphere.dae", nullptr, 
                                      SHADER_PHONG_BLINN_NO_TEXTURE, m_app->getFrustum(),
                                      m_render_context, math::vec3(1.0, 0.0, 0.0)));
    m_att_point_scale = math::identity_mat4();
    m_att_point_scale.m[0] = 0.25;
    m_att_point_scale.m[5] = 0.25;
    m_att_point_scale.m[10] = 0.25;

    check_gl_errors(true, "EditorRenderer::EditorRenderer");
}


EditorRenderer::~EditorRenderer(){

}


int EditorRenderer::render(struct render_buffer* rbuf){
    int num_rendered = 0;

    num_rendered = renderObjects(rbuf->buffer, rbuf->view_mat);

    check_gl_errors(true, "EditorRenderer::render");

    return num_rendered;
}


int EditorRenderer::renderObjects(const std::vector<object_transform>& buff, 
                                  const math::mat4& view_mat){
    int num_rendered = 0;

    m_render_context->useProgram(SHADER_PHONG_BLINN_NO_TEXTURE);
    glUniformMatrix4fv(m_pb_notex_view_mat, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(m_pb_notex_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    m_render_context->useProgram(SHADER_PHONG_BLINN);
    glUniformMatrix4fv(m_pb_view_mat, 1, GL_FALSE, view_mat.m);
    glUniformMatrix4fv(m_pb_proj_mat, 1, GL_FALSE, m_camera->getProjMatrix().m);

    for(uint i=0; i<buff.size(); i++){
        num_rendered += buff.at(i).object_ptr->render(buff.at(i).transform);
    }


    for(uint i=0; i<buff.size(); i++){
        BasePart* part = dynamic_cast<BasePart*>(buff.at(i).object_ptr.get());
        if(part)
            renderAttPoints(part, buff.at(i).transform);
        num_rendered += buff.at(i).object_ptr->render(buff.at(i).transform);
    }

    check_gl_errors(true, "EditorRenderer::renderObjects");

    return num_rendered;
}


void EditorRenderer::renderAttPoints(const BasePart* part, const math::mat4& body_transform){
    const std::vector<struct attachment_point>& att_points = part->getAttachmentPoints();

    math::mat4 att_transform;

    if(part->hasParentAttPoint()){
        const math::vec3& point = part->getParentAttachmentPoint().point;
        att_transform = body_transform * math::translate(math::identity_mat4(), point);
        m_att_point_model->setMeshColor(math::vec4(0.0, 1.0, 0.0, 1.0));
        m_att_point_model->render(att_transform * m_att_point_scale);
    }

    /*if(part->hasFreeAttPoint()){
        point = part->getFreeAttachmentPoint()->point;
        att_transform = body_transform * math::translate(math::identity_mat4(), point);
        m_att_point_model->setMeshColor(math::vec4(0.0, 1.0, 1.0, 1.0));
        num_rendered += m_att_point_model->render(att_transform * m_att_point_scale);
    }*/

    for(uint j=0; j<att_points.size(); j++){
        const math::vec3& point = att_points.at(j).point;
        att_transform = body_transform * math::translate(math::identity_mat4(), point);
        m_att_point_model->setMeshColor(math::vec4(1.0, 0.0, 0.0, 1.0));
        m_att_point_model->render(att_transform * m_att_point_scale);
    }
}