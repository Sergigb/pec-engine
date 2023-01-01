#ifndef PLANETRENDERER_HPP
#define PLANETRENDERER_HPP
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "BaseRenderer.hpp"
#include "../core/maths_funcs.hpp"
#include "../core/buffers.hpp"


class BaseApp;
class RenderContext;

#define SKYBOX_SIZE 100000.0f


class PlanetariumRenderer : public BaseRenderer{
    private:
        BaseApp* m_app;
        GLint m_debug_view_mat, m_debug_proj_mat, m_debug_color_location;
        RenderContext* m_render_context;
        GLuint m_vao, m_vbo_vert, m_vbo_tex, m_textures[6];
        GLint m_skybox_view_loc, m_skybox_proj_loc, m_skybox_model_loc;

        math::mat4 m_skybox_transforms[6];

        void renderPlanetariumOrbits(const std::vector<planet_transform>& buff, 
                                     const math::mat4& view_mat);
        void createSkybox();
        void renderSkybox(const math::mat4& view_mat);
    public:
        PlanetariumRenderer(BaseApp* app);
        ~PlanetariumRenderer();

        int render(struct render_buffer* rbuf);
};


#endif
