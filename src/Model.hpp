#ifndef MODEL_HPP
#define MODEL_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "Frustum.hpp"
#include "gl_utils.hpp"


class RenderContext;


class Model{
    private:
        int m_model_mat_location, m_color_location;
        GLuint m_vao, m_tex_id;
        GLuint m_vbo_vert, m_vbo_tex, m_vbo_ind, m_vbo_norm;
        float m_cs_radius;
        struct bbox m_aabb;
        int m_num_faces, m_tex_x, m_tex_y, m_n_channels;
        GLuint m_shader_programme;
        bool m_has_texture;
        math::vec4 m_mesh_color;

        const Frustum* m_frustum;
        const RenderContext* m_render_context;

        int loadScene(const std::string& pFile);
    public:
        Model();
        Model(const char* path_to_mesh, const char* path_to_texture, GLuint shader_programme, const Frustum* frustum, const RenderContext* render_context, const math::vec3& mesh_color);
        ~Model();

        void setMeshColor(const math::vec4& mesh_color);
        int render(const math::mat4& transform) const;
        void render_terrain(const math::mat4& transform) const;

};

#endif