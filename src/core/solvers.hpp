#ifndef SOLVERS_HPP
#define SOLVERS_HPP

struct particle_state;

// Some integrators to solve the motion equations. The two Euler ones are first order, the Verlet
// is second order. Maybe some day we should add higher order methods.

/*
 * Integrates the forces applied to a particle to find its next position using the Euler method.
 * It updates the velocity and the position of the particle. Check predictors.hpp to see the fields
 * of the particle state (basically origin, total force applied and initial velocity). Being a
 * first order solver, it does not use the previous origin and instead uses the velocity field
 * of the particle.
 *
 * @p: an instance of a particle to which some forces have been applied.
 * @delta_t: time interval used to solve the motion of the particle.
 */
inline void solverExplicitEuler(struct particle_state& p, double delta_t){
    dmath::vec3 acceleration = p.total_force / p.mass;
        
    dmath::vec3 velocity_tp1 = p.velocity + acceleration * delta_t;
    p.origin = p.origin + p.velocity * delta_t;

    p.velocity = velocity_tp1;
    p.total_force.v[0] = 0.0;
    p.total_force.v[1] = 0.0;
    p.total_force.v[2] = 0.0;
}


/*
 * Integrates the forces applied to a particle to find its next position using simplectic Euler. It
 * updates the velocity and the position of the particle. Check predictors.hpp to see the fields of
 * the particle state (basically origin, total force applied and initial velocity). Being a first
 * order solver, it does not use the previous origin and instead uses the velocity field of the
 * particle.
 *
 * @p: an instance of a particle to which some forces have been applied.
 * @delta_t: time interval used to solve the motion of the particle.
 */
inline void solverSymplecticEuler(struct particle_state& p, double delta_t){
    dmath::vec3 acceleration = p.total_force / p.mass;

    dmath::vec3 velocity_tp1 = p.velocity + acceleration * delta_t;
    p.origin = p.origin + velocity_tp1 * delta_t;

    p.velocity = velocity_tp1;
    p.total_force.v[0] = 0.0;
    p.total_force.v[1] = 0.0;
    p.total_force.v[2] = 0.0;
}


/*
 * Integrates the forces applied to a particle to find its next position using the Verler method.
 * It updates the velocity and the position of the particle. Check predictors.hpp to see the fields
 * of the particle state (basically origin, total force applied and initial velocity). Being a
 * second order solver, it uses the previous origin and the velocity field is not used.
 *
 * @p: an instance of a particle to which some forces have been applied.
 * @delta_t: time interval used to solve the motion of the particle.
 */
inline void solverVerlet(struct particle_state& p, double delta_t){
    dmath::vec3 prev_position = p.origin;

    dmath::vec3 acceleration = p.total_force / p.mass;
    p.origin = p.origin + (p.origin  - p.prev_origin) + acceleration * delta_t * delta_t;
    p.prev_origin = prev_position;

    p.total_force.v[0] = 0.0;
    p.total_force.v[1] = 0.0;
    p.total_force.v[2] = 0.0;
}


#endif