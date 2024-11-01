#ifndef EDITOR_RENDERER_HPP
#define EDITOR_RENDERER_HPP
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "BaseRenderer.hpp"
#include "../core/buffers.hpp"


class BaseApp;
class RenderContext;
class Camera;
class Model;


class EditorRenderer : public BaseRenderer{
    private:
        BaseApp* m_app;

        GLint m_pb_notex_view_mat, m_pb_notex_proj_mat;
        GLint m_pb_view_mat, m_pb_proj_mat;

        RenderContext* m_render_context;
        const Camera* m_camera;

        std::unique_ptr<Model> m_att_point_model;
        std::unique_ptr<Model> m_grid;
        math::mat4 m_att_point_scale;

        int renderObjects(const std::vector<object_transform>& buff, 
                          const math::mat4& view_mat);
        void renderAttPoints(const BasePart* part, const math::mat4& body_transform);
    public:
        EditorRenderer(BaseApp* app);
        ~EditorRenderer();

        int render(struct render_buffer* rbuf);
};


#endif
