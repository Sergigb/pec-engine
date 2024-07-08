#include "solvers.hpp"
#include "predictors.hpp"


inline void solverExplicitEuler(struct particle_state& p, double delta_t){
    dmath::vec3 acceleration = p.total_force / p.mass;
        
    dmath::vec3 velocity_tp1 = p.velocity + acceleration * delta_t;
    p.origin = p.origin + p.velocity * delta_t;

    p.velocity = velocity_tp1;
    p.total_force.v[0] = 0.0;
    p.total_force.v[1] = 0.0;
    p.total_force.v[2] = 0.0;
}


inline void solverSymplecticEuler(struct particle_state& p, double delta_t){
    dmath::vec3 acceleration = p.total_force / p.mass;

    dmath::vec3 velocity_tp1 = p.velocity + acceleration * delta_t;
    p.origin = p.origin + velocity_tp1 * delta_t;

    p.velocity = velocity_tp1;
    p.total_force.v[0] = 0.0;
    p.total_force.v[1] = 0.0;
    p.total_force.v[2] = 0.0;
}


inline void solverVerlet(struct particle_state& p, double delta_t){
    dmath::vec3 prev_position = p.origin;

    dmath::vec3 acceleration = p.total_force / p.mass;
    p.origin = (p.origin - p.prev_origin) + acceleration * delta_t;
    p.prev_origin = prev_position;

    p.total_force.v[0] = 0.0;
    p.total_force.v[1] = 0.0;
    p.total_force.v[2] = 0.0;
}
