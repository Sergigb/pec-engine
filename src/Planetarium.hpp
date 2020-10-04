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


struct surface_node{
    dmath::vec3 patch_translation;
    math::vec2 tex_shift;
    double scale; // scale = 1/depth
    dmath::versor base_rotation;
    std::unique_ptr<struct surface_node> childs[4];
    bool has_texture, texture_loaded, loading, data_ready;
    bool has_elevation;
    short level, x, y;
    char side, tiks_since_last_use;
    GLuint tex_id, tex_id_lod;
    GLuint e_tex_id, e_tex_id_lod;
    unsigned char* data;
    int tex_x, tex_y;
    float texture_scale;
    struct surface_node* uppermost_textured_parent;
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

        void render_side(struct surface_node& surface_subtree, math::mat4& planet_transform, int level, dmath::vec3& cam_origin, double sea_level);
    public:
        Planetarium();
        Planetarium(int gl_width, int gl_height);
        ~Planetarium();

        void run();
};


#endif
