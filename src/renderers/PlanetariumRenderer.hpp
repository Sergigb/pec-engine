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

        void renderPlanetariumOrbits(const std::vector<planet_transform>& buff, 
                                     const math::mat4& view_mat);
    public:
        PlanetariumRenderer(BaseApp* app);
        ~PlanetariumRenderer();

        void render(struct render_buffer* rbuf);
};


#endif
