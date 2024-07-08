#ifndef PREDICTORS_HPP
#define PREDICTORS_HPP

#include "maths_funcs.hpp"

#define MAX_CONIC_SOLVER_ITER 10

/*
 * This class defines a series of functions and data structures that can be used to predict orbital
 * characteristics. One example of the currently implemented one is the simple orbital predictor,
 * which predicts the orbit of a series of queried particles.
 */

struct orbital_data;


/*
 * This struct contains the state of a particle, which includes its origin, its velocity and total
 * force before its position is final position is integrated.
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


inline dmath::vec3& computePlanetPosition(const orbital_data& data, double time);


#endif