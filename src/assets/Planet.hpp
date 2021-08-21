#ifndef PLANET_HPP
#define PLANET_HPP


#include "../core/maths_funcs.hpp"
#include "PlanetTree.hpp"


class RenderContext;


#define NUM_VERTICES 300

#define MAX_SOLVER_ITER 10


/* Everything in this struct is too verbose and it bothers me... */

struct orbital_data{
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
        orbital_data m_orbital_data;
        std::uint32_t m_id;
        std::string m_name;

        // orbit render buffers
        GLuint m_vao, m_vbo_vert, m_vbo_ind;

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

        void setID(std::uint32_t id);
        void setName(const char* name);

        const dmath::vec3& getPosition() const;
        const dmath::vec3& getPrevPosition() const;
        const std::string& getName() const;
        const orbital_data& getOrbitalData() const;
        orbital_data& getOrbitalData();
        const dmath::mat4 getTransform() const;
        std::uint32_t getId() const;
};


#endif
