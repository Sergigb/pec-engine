#ifndef PLANETARIUM_HPP
#define PLANETARIUM_HPP

#include <GL/glew.h>
#include <memory>

#include "BaseApp.hpp"


#define SIDE_PX 1
#define SIDE_NX 2
#define SIDE_PY 3
#define SIDE_NY 4
#define SIDE_PZ 5
#define SIDE_NZ 6


struct surface_node{
    dmath::vec3 patch_translation;
    math::vec2 tex_shift;
    double scale; // scale = 1/depth
    dmath::versor base_rotation;
    std::unique_ptr<struct surface_node> childs[4];
    bool has_texture;
    short level;
    char side;
    GLuint tex_id;
};


struct planet_surface{
    struct surface_node surface_tree[6];
    short max_levels;
    double planet_sea_level;
};


class Planetarium : public BaseApp{
    private:
        void init();

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        void render_side(const struct surface_node& surface_subtree, Model& model, math::mat4& planet_transform, int level, dmath::vec3& cam_origin, double sea_level);
    public:
        Planetarium();
        Planetarium(int gl_width, int gl_height);
        ~Planetarium();

        void run();
};


#endif
