#ifndef PLANETARIUM_HPP
#define PLANETARIUM_HPP

#include "BaseApp.hpp"


struct surface_node{
    dmath::vec3 patch_translation;
    double scale; // scale = 1/depth
    dmath::versor base_rotation;
    math::vec4 color;
    //struct surface_node surface_subtree[4];
    bool is_leaf;
};


struct planet_surface{
    struct surface_node surface_tree[6];
};


class Planetarium : public BaseApp{
    private:
        void init();

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        void render_side(const struct surface_node& surface_subtree, GLuint shader, GLuint relative_planet_location, Model& model, math::mat4& planet_transform);
    public:
        Planetarium();
        Planetarium(int gl_width, int gl_height);
        ~Planetarium();

        void run();
};


#endif
