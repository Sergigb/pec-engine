#ifndef PLANET_HPP
#define PLANET_HPP


#include "../core/maths_funcs.hpp"
#include "PlanetTree.hpp"


class RenderContext;


#define NUM_VERTICES 300

#define MAX_SOLVER_ITER 10


/* Everything in this struct is too verbose and it bothers me... */

struct orbital_data{
    std::uint32_t id;
    std::string name;
    // initially read from file
    double mass, radius, semi_major_axis_0, semi_major_axis_d, eccentricity_0, eccentricity_d,
           inclination_0, inclination_d, mean_longitude_0, mean_longitude_d, longitude_perigee_0,
           longitude_perigee_d, long_asc_node_0, long_asc_node_d;
    // updated at each time step
    double semi_major_axis, eccentricity, inclination, mean_longitude, longitude_perigee, long_asc_node;
    // derivated
    double mean_anomaly, arg_periapsis, eccentric_anomaly, true_anomaly, period;

    dmath::vec3 pos, pos_prev;
};

class Planet{
    private:
        dmath::mat4 m_planet_transform; // this will change
        orbital_data m_orbital_data;

        // orbit render buffers
        GLuint m_vao, m_vbo_vert, m_vbo_ind;
        //GLuint m_color_location, m_proj_location, m_view_location; MOVE THIS TO PlanetarySystem!!
        /*
        m_color_location = m_render_context->getUniformLocation(SHADER_DEBUG, "line_color");
        m_proj_location = m_render_context->getUniformLocation(SHADER_DEBUG, "proj");
        m_view_location = m_render_context->getUniformLocation(SHADER_DEBUG, "view");
        */
        RenderContext* m_render_context;

        PlanetTree m_planet_tree;

        void initBuffers();
    public:
        Planet(RenderContext* render_context);
        ~Planet();

        void render(const dmath::vec3& cam_translation, const dmath::mat4 transform);
        void render(const dmath::vec3& cam_translation);
        void renderOrbit() const;

        void buildSurface();
        void destroySurface();

        void updateOrbitalElements(const double cent_since_j2000);
        void updateRenderBuffers(double current_time);

        dmath::mat4& getTransform();
        dmath::mat4 getTransform() const;
};


#endif
