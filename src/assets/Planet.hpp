#ifndef PLANET_HPP
#define PLANET_HPP

#include <vector>
#include <memory>


#include "../core/maths_funcs.hpp"
#include "PlanetTree.hpp"


class RenderContext;
class Kinematic;
class Sprite;


#define NUM_VERTICES 300

#define MAX_SOLVER_ITER 10


/* 
 * Struct with orbital data of a celestial body. These parameters are used to solve the two-body 
 * problem. Since we don't suppot moons yet, we solve the two-body problems between the planets and
 * the sun. For parameters that are continuously updated we have their value in the current 
 * timestep the initial value (at the start of the epoch), and its derivative.
 *
 * @m: mass of the body, in kgs.
 * @r: mean radius of the body. In meters.
 * @a: value of the semi-major axis. In AU.
 * @a_0: initial value of the semi-major axis. In AU.
 * @a_d: derivative of the semi-major axis. In AU/century.
 * @e: eccentricity of the orbit.
 * @e_0: initial value of the eccentricity of the orbit.
 * @e_d: derivative of the eccentricity.
 * @i: inclination of the orbit, in rads.
 * @i_0: initial value of the inclination of the orbit, in rads.
 * @i_d: derivative of the inclination, in rads/century.
 * @L: mean longitude of the orbit, L = w + M. In rads.
 * @L_0: initial value of the  mean longitude of the orbit. In rads.
 * @L_d: derivative of the mean longitude. In rads/century.
 * @p: longitude of the periapsis. In rads.
 * @p_0: initial value of the longitude of the periapsis. In rads.
 * @p_d: derivative of the longitude of the periapsis. In rads/century.
 * @W: longitude of the ascending node, in rads.
 * @W_0: initial value of the longitude of the ascending node, in rads.
 * @W_d: derivative of the ascending node, in rads/century.
 * @M: mean anomaly. In rads.
 * @w: argument of the periapsis, in rads.
 * @E: eccentric anomaly, in rads.
 * @v: true anomaly, in rads.
 * @period: period of the orbit, in centuries.
 * @pos: current position of the body.
 * @pos_prev: position of the body in the previous tick.
 */

struct orbital_data{
    // initially read from file
    double m, r, a_0, a_d, e_0, e_d, i_0, i_d, L_0, L_d, p_0, p_d, W_0, W_d;

    // updated at each time step
    double a, e, i, L, p, W;
    // derivated
    double M, w, E, v, period;

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
        std::string m_thumbnail_path;
        math::vec3 m_base_color;

        PlanetTree m_planet_tree;

        std::vector<Kinematic*> m_kinematics;

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
         * Renders the orbit of the planet, used in the planetarium view. The shaders has to have 
         * been bound before calling this method.
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
         * Registers a kinematic on this planet. The kinematic will be transformed relative to the
         * planet. Planets do not currently have the ability to add or remove kinematics, such
         * feature would be useful to remove bodies that are far away from the player in order to
         * improve efficiency. In such case, it would be better to keep the kinematics of the 
         * AssetManager in a map.
         * 
         * @kinematic: raw pointer to the kinematic, so it's not owned by the planet.
         */
        void registerKinematic(Kinematic* kinematic);

        /*
         * Sets the path to the planet thumbnail.
         *
         * @path: path to the thumbnail file.
         */
        void setThumbnailPath(const char* path);

        /*
         * Returns the path to the planet thumbnail.
         */
        const char* getThumbnailPath() const;

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
         * Returns the transform of the planet in double precision.
         */
        const dmath::mat4 getTransform() const;

        /*
         * Returns the ID of the planet.
         */
        std::uint32_t getId() const;

        /*
         * Updates the kinematic objects of the planet.
         */
        void updateKinematics();

        /*
         * Sets the line color to be used when rendering the planet orbit in the planetarium.
         *
         * @color: vec3 with the RGB color values.
         */
        void setBaseColor(const math::vec3& color);

        /*
         * Returns the line color to be used when rendering the planet orbit in the planetarium.
         */
        const math::vec3& getBaseColor() const;
};


#endif
