#ifndef MODEL_HPP
#define MODEL_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "../core/Frustum.hpp"
#include "../core/utils/gl_utils.hpp"


class RenderContext;


/*
 * Model class, holds a 3D model. Quite simple but ok for now.
 */
class Model{
    private:
        int m_model_mat_location, m_color_location, m_shader;
        GLuint m_vao, m_tex_id;
        GLuint m_vbo_vert, m_vbo_tex, m_vbo_ind, m_vbo_norm;
        float m_cs_radius;
        struct bbox m_aabb;
        int m_num_faces, m_tex_x, m_tex_y, m_n_channels;
        bool m_has_texture;
        math::vec4 m_mesh_color;

        static const Frustum* m_frustum;
        static const RenderContext* m_render_context;

        friend void setStaticMembers(const Frustum* frustum,
                                     const RenderContext* render_context);

        int loadScene(const std::string& pFile);
    public:
        Model();
        /*
         * Constructor.
         *
         * @path_to_mesh: char string with the path to the mesh file.
         * @path_to_texture: char string with the path to the mesh texture.
         * @shader: shader to be used to render this model (see RenderContext, macros SHADER_*)
         * @mesh_color: color of the mesh.
         */
        Model(const char* path_to_mesh, const char* path_to_texture,
              int shader, const math::vec3& mesh_color);
        ~Model();

        /*
         * Sets the color of the mesh.
         *
         * @mesh_color: new color of the mesh.
         */
        void setMeshColor(const math::vec4& mesh_color);

        /*
         * Render method.
         *
         * @transform: transform matrix of the model.
         */
        int render(const math::mat4& transform) const;

        /*
         * Special method for when we are rendering terrain.
         *
         * @transform: transform matrix of the model.
         */
        void render_terrain(const math::mat4& transform) const;

        /*
         * Sets the static pointers to the frustum and render context.
         *
         * @frustum: pointer to the frustum object of the app.
         * @render_context: pointer to the recnder context object of the app.
         */
        static void setStaticMembers(const Frustum* frustum,
                                     const RenderContext* render_context);
};

#endif