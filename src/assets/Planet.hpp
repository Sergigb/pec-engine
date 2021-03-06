#ifndef PLANET_HPP
#define PLANET_HPP

#include <memory>
#include <vector>
#include <mutex>

#include <GL/glew.h>
#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "../core/maths_funcs.hpp"


class Model;
class Frustum;
class RenderContext;


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
    math::vec2 tex_shift, tex_shift_lod;
    double scale; // scale = 1/depth
    dmath::versor base_rotation;
    std::unique_ptr<struct surface_node> childs[4];
    bool has_texture, texture_loaded, loading, data_ready;
    bool has_elevation;
    short level, x, y;
    char side, ticks_since_last_use;
    GLuint tex_id, tex_id_fl; // tex_id_fl, texture id first level, used for lod
    GLuint e_tex_id, e_tex_id_fl;
    unsigned char* data, * data_elevation;
    int tex_x, tex_y, e_tex_x, e_tex_y;
    float texture_scale, texture_scale_lod;
    struct surface_node* uppermost_textured_parent;

    ~surface_node(){
        if(childs[0]){
            childs[0].reset();
            childs[1].reset();
            childs[2].reset();
            childs[3].reset();
        }
    }
};


struct planet_surface{
    struct surface_node surface_tree[6];
    short max_levels;
    double planet_sea_level;

    std::vector<struct surface_node*> bound_nodes;
    std::vector<struct surface_node*> data_ready_nodes;
    std::mutex data_ready_mtx;
};



class Planet{
    private:
        static std::unique_ptr<Model> m_base32;
        static std::unique_ptr<Model> m_base64;
        static std::unique_ptr<Model> m_base128;

        std::vector<struct surface_node*> bound_nodes;
        std::vector<struct surface_node*> data_ready_nodes;
        std::mutex data_ready_mtx;

        struct planet_surface m_surface;

        RenderContext* m_render_context;

        dmath::mat4 m_planet_transform; // this will change

        GLuint m_relative_planet_location, m_texture_scale_location, m_tex_shift_location, m_planet_radius_location;
        GLuint m_planet_texture, m_elevation_texture;

        void render_side(struct surface_node& node, math::mat4& planet_transform_world, int max_level, const dmath::vec3& cam_origin, double sea_level);
    public:
        Planet(RenderContext* render_context);
        ~Planet();

        void render(const dmath::vec3& cam_translation);
        void render(const dmath::vec3& cam_translation, const dmath::mat4 transform);

        dmath::mat4& getTransform();
        dmath::mat4 getTransform() const;

        static void loadBases(const Frustum* frustum, const RenderContext* render_context);
};


#endif
