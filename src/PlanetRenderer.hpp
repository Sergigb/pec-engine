#ifndef PLANET_RENDERER_HPP
#define PLANET_RENDERER_HPP

#include <GL/glew.h>
#include <memory>
#include <vector>

#include "core/BaseApp.hpp"


#define SIDE_PX 0
#define SIDE_NX 1
#define SIDE_PY 2
#define SIDE_NY 3
#define SIDE_PZ 4
#define SIDE_NZ 5

#define TEXTURE_LOCATION 0
#define ELEVATION_LOCATION 1


class FontAtlas;


class PlanetRenderer : public BaseApp{
    private:
        void init();

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;
        bool m_polygon_mode_lines;

        void render_side(struct surface_node& surface_subtree, math::mat4& planet_transform, int level, dmath::vec3& cam_origin, double sea_level);
    public:
        PlanetRenderer();
        PlanetRenderer(int gl_width, int gl_height);
        ~PlanetRenderer();

        void run();
        void processInput();
        void terminate();
};


#endif
