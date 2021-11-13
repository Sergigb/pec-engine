#ifndef PLANET_TREE_HPP
#define PLANET_TREE_HPP

#include <memory>
#include <vector>
#include <mutex>

#include <GL/glew.h>
#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "../core/maths_funcs.hpp"

#define SIDE_PX 0
#define SIDE_NX 1
#define SIDE_PY 2
#define SIDE_NY 3
#define SIDE_PZ 4
#define SIDE_NZ 5


#define TEXTURE_LOCATION 0
#define ELEVATION_LOCATION 1

/*
 There are a couple of things that we don't check here. First, when we destroy the surface, we don't
 check if we have threads loading textures in the background. We should have a conditional variable
 and a counter to control this.

 Also, when we are not using asynchronous texture loading, we don't clear the "data_ready" vector
 because we just call bindLoadedTexture. We don't need to add the node in this vector when we are
 not using threads.
*/


/*
 * Node object of the surface tree. Contains (for now) the necessary stuff to render the surface
 * of the planet. At some point it will need to hold the necessary structures of the terrain
 * collision. This is perhaps the most convoluted part of this project so far and I don't really
 * remember what every variable of this struct does, but at some point it needs to be better
 * documented.
 */
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


/*
 * Struct with the surface tree of the planet. Each planet has one of these.
 *
 * @is_built: true if the tree is built.
 * @surface_node: the first eight children of the tree. We have eight children because we use a
 * quadrilateralized spherical cube to render the planet, so each node is one side of the cube.
 * @max_levels: max depth of the tree, more levels = more detail, but more memory consumption.
 * @planet_sea_level: sea level of the planet (in meters).
 * @bound_nodes: a vector with the nodes that have bound textures. Used to free them upon
 * destruction of the tree.
 * @data_ready_nodes: a vector with the nodes that have loaded the textures but they have been
 * not bound, used when we are loading the textures in a different thread and we need to bind them
 * @data_ready_mtx: lock around data_ready_nodes.
 */
struct planet_surface{
    bool is_built;
    struct surface_node surface_tree[6];
    short max_levels;
    double planet_sea_level;

    std::vector<struct surface_node*> bound_nodes;
    std::vector<struct surface_node*> data_ready_nodes;
    std::mutex data_ready_mtx;
};


class Model;
class Frustum;
class RenderContext;
class Planet;


/*
 * Essentially holds the planet surface and has the necessary methods to build the tree, load the
 * textures etc.
 */
 class PlanetTree{
    private:
        static std::unique_ptr<Model> m_base32;
        static std::unique_ptr<Model> m_base64;
        static std::unique_ptr<Model> m_base128;

        std::vector<struct surface_node*> bound_nodes;
        std::vector<struct surface_node*> data_ready_nodes;
        std::mutex data_ready_mtx;

        struct planet_surface m_surface;

        RenderContext* m_render_context;
        Planet* m_planet;

        GLuint m_relative_planet_location, m_texture_scale_location,
               m_tex_shift_location, m_planet_radius_location;
        GLuint m_planet_texture, m_elevation_texture;

        /*
         * Sets the local transform of the node.
         *
         * @node: the surface node.
         * @parent: the parent of the previous node.
         * @sign_side_1: Sign that is used to decide to shift left or right the node.
         * @sign_side_2: idem but up and down.
         */
        void setTransform(struct surface_node& node, const struct surface_node& parent,
                          int sign_side_1, int sign_side_2);

        /*
         * Binds the texture of a node. The texture should have been loaded.
         *
         * @node: the node.
         */
        void bindTexture(struct surface_node& node);

        /*
         * Binds the elevation texture of the node.
         *
         * @node: the node.
         */
        void bindElevationTexture(struct surface_node& node);

        /*
         * Builds the children of a node. This method is called recursively to build each side of
         * the cube.
         * @node: parent node.
         * @num_levels: depth of the tree thus far.
         */
        void buildChilds(struct surface_node& node, int num_levels);

        /*
         * Binds the loaded surface and elevation textures of the given node (held in pointers data
         * and data_elevation). This method has to be called from the thread that holds the OpenGL
         * context. The node will be pushed into the bound_nodes vector of the surface.
         *
         * @node: reference to the node that holds the data and the texture ids where they are
         * going to be bound
         */
        void bindLoadedTexture(struct surface_node& node);

        /*
         * Loads the surface and elevation textures into the data and data_elevation pointers. This
         * method can be called asynchronously to load the data with std::thread. After loading the
         * texture you can call bindLoadedTexture to bind them, but this has to happen
         * synchronously. The node will be put into the data_ready vector inside the surface struct.
         *
         * @node: pointer to the node we want to load the textures for.
         * @surface: pointer to the surface object, we can probably just use the member object...
         */
        void asyncTextureLoad(struct surface_node* node, struct planet_surface* surface);

        /*
         * Binds all the textures inside data_ready. Like bindLoadedTexture has to be called from
         * the thread that holds the OpenGL context.
         */
        void bindLoadedTextures();

        /*
         * Frees all the textures that haven't been used for the last 100 ticks.
         */
        void textureFree();

        /*
         * Clears a side of the spherical cube. It frees bound and loaded textures.
         *
         * @node: reference to the node we want to clear.
         * @num_levels: to avoid cleaning lower levels with no texture.
         */
        void cleanSide(struct surface_node& node, int num_levels);

        /*
         * Recursive method that renders one side of the cube. Uses the proximity of the player
         * to know how deep it has to go inside the planet tree. The current node decides to
         * render itself if it's far away enough from the player, this is how I've implemented
         * LOD.
         *
         * @node: current node of the tree.
         * @planet_transform_world: global transform of the planet wrt the centered camera, in
         * single precision.
         * @max_level: maximum surface levels, should not be a parameter, just look at the
         * surface...
         * @cam_origin: origin of the camera, in double precision.
         * @sea_level: sea level, used to know how far away are from the surface (well, the sea
         * level actually...)
         */
        void renderSide(struct surface_node& node, const math::mat4& planet_transform_world,
                        int max_level, const dmath::vec3& cam_origin, double sea_level);
    public:
        PlanetTree();
        /*
         * Constructor
         *
         * @render_context: pointer to the render context object
         * @planet: pointer to the planet object that owns the tree.
         */
        PlanetTree(RenderContext* render_context, Planet* planet);
        ~PlanetTree();

        /*
         * Builds the surface tree.
         */
        void buildSurface();

        /*
         * Render the planet.
         *
         * @cam_translation: global camera origin.
         * @transform: transform of the planet.
         */
        void render(const dmath::vec3& cam_translation, const dmath::mat4 transform);

        /*
         * Static method to load the grids used to render the surface.
         *
         * @frustum: pointer to the game's frustum object. Not used yet to cull the rendering.
         * @render_context: pointer to the render context object.
         */
        static void loadBases(const Frustum* frustum, const RenderContext* render_context);
};


#endif


