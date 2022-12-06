#ifndef PLANETRENDERER_HPP
#define PLANETRENDERER_HPP
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "BaseRenderer.hpp"
#include "../core/maths_funcs.hpp"
#include "../core/buffers.hpp"


class BaseApp;


class PlanetariumRenderer : public BaseRenderer{
    private:
        BaseApp* m_app;
        GLint m_debug_view_mat, m_debug_proj_mat, m_debug_color_location;

        void renderPlanetariumOrbits(const std::vector<planet_transform>& buff, 
                                     const math::mat4& view_mat);
    public:
        PlanetariumRenderer(BaseApp* app);
        ~PlanetariumRenderer();

        void render(struct render_buffer* rbuf);
};


#endif
