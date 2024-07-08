#ifndef PREDICTORS_HPP
#define PREDICTORS_HPP

#include <memory>

#include "maths_funcs.hpp"
#include "../assets/PlanetarySystem.hpp"

#define MAX_CONIC_SOLVER_ITER 10

/*
 * This class defines a series of functions and data structures that can be used to predict orbital
 * characteristics. One example of the currently implemented one is the simple orbital predictor,
 * which predicts the orbit of a series of queried particles.
 */

struct orbital_data;


/*
 * This struct contains the motion state of a particle, which includes its origin, its velocity and
 * total force before its position is integrated. prev_origin contains the previous location, used
 * by second-order integrators.
 */
struct particle_state{
    dmath::vec3 origin;
    dmath::vec3 prev_origin;
    dmath::vec3 velocity;
    dmath::vec3 total_force;
    double mass;

    particle_state(){}

    particle_state(const dmath::vec3& o, const dmath::vec3& v, btScalar m){
        origin = o;
        prev_origin = o;
        velocity = v;
        mass = m;
        total_force = dmath::vec3(0.0, 0.0, 0.0);
    }

    void operator=(const struct particle_state& p){
        origin = p.origin;
        prev_origin = p.prev_origin;
        velocity = p.velocity;
        mass = p.mass;
        total_force = dmath::vec3(0.0, 0.0, 0.0);
    }

    particle_state(const struct particle_state& p){
        origin = p.origin;
        prev_origin = p.prev_origin;
        velocity = p.velocity;
        mass = p.mass;
        total_force = dmath::vec3(0.0, 0.0, 0.0);
    }
};


/*
 * Returns the position of the planet with the parameters given in data (check Planet.hpp) at the
 * queried time.
 * 
 * @data: the orbital parameters of the planet (struct defined in Planet.hpp).
 * @time: the time at which we want to know the position. In centuries.
 */
inline dmath::vec3& compute_planet_position(const orbital_data& data, double time);


/*
 * This function computes the trajectories of different particles given their initial motion state
 * and a planetary system object. These predictions are used specifically to render these orbits,
 * so the location predictions are stored as floats. You can specify the starting time of the 
 * simulation (which determines the initial position of the planets), the number of steps, the
 * time delta of each step, and the scale of the stored coordinates. The precision of the
 * predictions depends on the time delta of each step and the number of steps.
 * 
 * @planet_system: planetary system object (check assets/PlanetarySystem.hpp) with the system data
 * @position_buffers: reference to a vector of vectors of float (does not need to be initialised).
 * Each buffer belongs to a particle, and will contiguously contain the 3D location of the particle
 * at each time step, the components of the coordinates are also contiguous (x1, y1, z1, x2, y2, 
 * z2, etc). These buffers can directly be used to render the trajectories of the praticles.
 * @states: the initial motion states of each particle.
 * @start_time: starting time of the simulation, determines the location of the planets, and should
 * be the current time of the simulation.
 * @predictor_delta_t_secs: time delta of each step of the simulation.
 * @predictor_steps: number of steps of the simluation, in seconds.
 * @scale: divides each coordinate.
 */
inline void compute_trajectories_render(const PlanetarySystem* planet_system,
                                        std::vector<std::vector<GLfloat>>& position_buffers,
                                        std::vector<struct particle_state>& states,
                                        double start_time; uint predictor_delta_t_secs,
                                        uint predictor_steps, float scale);


/*
 * This function computes the approximate trajectories of different particles given their initial
 * motion state and a planetary system object. Stores the positions of each particle at each time
 * as double precision coordinates. You can specify the starting time of the simulation (which
 * determines the initial position of the planets), the number of steps, the time delta of each
 * step, and the scale of the stored coordinates. The precision of the predictions depends on the
 * time delta of each step and the number of steps.
 * 
 * @planet_system: planetary system object (check assets/PlanetarySystem.hpp) with the system data
 * @positions: vector of vectors of 3D coordinates, they store the location of each particle at
 * each step of the simluation.
 * @states: the initial motion states of each particle.
 * @start_time: starting time of the simulation, determines the location of the planets, and should
 * be the current time of the simulation.
 * @predictor_delta_t_secs: time delta of each step of the simulation.
 * @predictor_steps: number of steps of the simluation, in seconds.
 * @scale: divides each coordinate.
 */
inline void compute_trajectories_double(const PlanetarySystem* planet_system,
                                        std::vector<std::vector<dmath::vec3>>& positions,
                                        std::vector<struct particle_state>& states,
                                        double start_time; uint predictor_delta_t_secs,
                                        uint predictor_steps)


#endif