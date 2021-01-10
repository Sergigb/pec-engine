#ifndef PLANETARIUM_HPP
#define PLANETARIUM_HPP

#include <GL/glew.h>
#include <memory>
#include <vector>

#include "BaseApp.hpp"


#define SIDE_PX 0
#define SIDE_NX 1
#define SIDE_PY 2
#define SIDE_NY 3
#define SIDE_PZ 4
#define SIDE_NZ 5

#define TEXTURE_LOCATION 0
#define ELEVATION_LOCATION 1


class FontAtlas;


class Planetarium : public BaseApp{
    private:
        void init();

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        void render_side(struct surface_node& surface_subtree, math::mat4& planet_transform, int level, dmath::vec3& cam_origin, double sea_level);
    public:
        Planetarium();
        Planetarium(int gl_width, int gl_height);
        ~Planetarium();

        void run();
};


#endif
