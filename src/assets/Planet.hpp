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


/*
 * Planet class with all the good stuff to simulate a planetary system. Also contains a PlanetTree,
 * which is used to render and hold the collision of the terrain.
 */
class Planet{
    private:
        orbital_data m_orbital_data;
        std::uint32_t m_id;
        std::string m_name;

        /* orbit render buffers */
        GLuint m_vao, m_vbo_vert, m_vbo_ind;

        RenderContext* m_render_context;

        PlanetTree m_planet_tree;

        void initBuffers();
    public:
        /*
         * Constructor
         *
         * @render_context: pointer to the render context object
         */
        Planet(RenderContext* render_context);
        ~Planet();

        /*
         * Render method. It requires the double cam translation because the rendering is quite
         * complex.
         *
         * @cam_translation: translation of the camera (not relative to anything).
         */
        void render(const dmath::vec3& cam_translation);

        /*
         * Similar to the other render method but in this one the transform of the planet can be
         * overrided.
         *
         * @cam_translation: translation of the camera (not relative to anything).
         * @transform: transform used to render the planet.
         */
        void render(const dmath::vec3& cam_translation, const dmath::mat4 transform);

        /*
         * Renders the orbit of the planet, used in the planetarium view.
         */
        void renderOrbit() const;

        /*
         * Builds the surface tree (contained in PlanetTree) used to render and, at some point in
         * the future, in the collision. This tree is quite big in memory (30MB+?), and should not
         * be built for all the planets at the same time, just for the one we are in right now.
         */
        void buildSurface();

        /*
         * Destroys the surface/collision tree.
         */
        void destroySurface();

        /*
         * Updates the orbital elements of the planet, should be called at each tick if the game is
         * not paused.
         *
         * @cent_since_j2000: centuries passed since the reference epoch, which is j2000 if we're
         * in the solar system and we're using that reference epoch in particular. The unit is a
         * century.
         */
        void updateOrbitalElements(const double cent_since_j2000);

        /*
         * Updates the render buffers for the drawing of the orbit (renderOrbit). We shouldn't need
         * to call this method each tick, since the orbits barely change in a year.
         *
         * @current_time: current time in centuries.
         */
        void updateRenderBuffers(double current_time);

        /*
         * Sets the ID of the planet.
         *
         * @id: new ID of the planet.
         */
        void setID(std::uint32_t id);

        /*
         * Sets the name of the planet.
         *
         * @name: name of the planet
         */
        void setName(const char* name);

        /*
         * Returns the position of the planet.
         */
        const dmath::vec3& getPosition() const;

        /*
         * Returns the position of the planet in the previous tick, can be useful to compute the
         * speed of the planet without deriving anything, but it shouldn't be used with big
         * timesteps.
         */
        const dmath::vec3& getPrevPosition() const;

        /*
         * Returns the name of the planet.
         */
        const std::string& getName() const;

        /*
         * Returns a reference to the struct with the orbital parameters (orbital_data) of the
         * planet.
         */
        const orbital_data& getOrbitalData() const;

        /*
         * Non const version of getOrbitalData, used to load the orbital parameters.
         */
        orbital_data& getOrbitalData();

        /*
         * Returns a reference to the transform of the planet.
         */
        const dmath::mat4 getTransform() const;

        /*
         * Returns the ID of the planet.
         */
        std::uint32_t getId() const;
};


#endif
