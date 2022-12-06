#ifndef SIMULATION_RENDERER_HPP
#define SIMULATION_RENDERER_HPP
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "BaseRenderer.hpp"
#include "../core/maths_funcs.hpp"
#include "../core/buffers.hpp"


class BaseApp;
class RenderContext;
class Camera;


class SimulationRenderer : public BaseRenderer{
    private:
        BaseApp* m_app;

        GLint m_pb_notex_view_mat, m_pb_notex_proj_mat;
        GLint m_pb_view_mat, m_pb_proj_mat;
        GLint m_planet_view_mat, m_planet_proj_mat;

        RenderContext* m_render_context;
        const Camera* m_camera;

        int renderObjects(const std::vector<object_transform>& buff, const math::mat4& view_mat);
    public:
        SimulationRenderer(BaseApp* app);
        ~SimulationRenderer();

        int render(struct render_buffer* rbuf);
};


#endif
