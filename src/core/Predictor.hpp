#ifndef PREDICTOR_HPP
#define PREDICTOR_HPP

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "maths_funcs.hpp"
#include "../assets/Planet.hpp"


#define MAX_CONIC_SOLVER_ITER 10

class BaseApp;
class PlanetarySystem;


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


struct fixed_time_trajectory_config{
    double predictor_start_time;
    double predictor_period_secs;
    int predictor_steps;
    float predictor_scale;

    fixed_time_trajectory_config(){
        predictor_start_time = 0;
        predictor_period_secs = 31536000.0;
        predictor_steps = 400;
        predictor_scale = 1.0;
    }

    fixed_time_trajectory_config(double start_time, double period_secs,
                                 int steps, float scale){
        predictor_start_time = start_time;
        predictor_period_secs = period_secs;
        predictor_steps = steps;
        predictor_scale = scale;
    }
};


class Predictor{
    private:
        const BaseApp* m_app;
    public:
        Predictor(const BaseApp* app);
        ~Predictor();

        /*
         * Computes the cartesian origin and velocity of an object given its orbital parameters.
         * Only parameters a_0, e_0, i_0, L_0, W_0, p_0 and a_d, e_d, i_d, L_d, W_d, p_d are
         * necessary. If we want to match the velocity of the target body, we also use the target
         * mass. 
         * 
         * @data: orbital parameters of the object's orbit (struct defined in Planet.hpp).
         * @body_target: id of the planet that we want to compute the orbit relative to. If 0, the
         * object will orbit around the star.
         * @time: time at which we want to compute the position and velocity.
         * @match_frame: if true, the speed of the object will be computed taking into acoount the
         * frame of reference of the target. If we want to orbit the star, this parameter is
         * irrelevant. 
         * @origin: will contain the cartesian coordinates of the object.
         * @velocity: will contain the velocity of the object.
         */
        void computeObjectPosVel(const orbital_data& data, std::uint32_t body_target, 
                                 double time, bool match_frame, dmath::vec3& origin,
                                 dmath::vec3& velocity) const;

        /*
         * Calculates the position of the object with the parameters given in data (check Planet.hpp) at
         * the queried time.
         * 
         * @data: the orbital parameters of the object (struct defined in Planet.hpp).
         * @time: the time at which we want to know the position. In centuries.
         * @planet_origin: is updated with the position of the planet at the queried time
         */
        void computeObjectPos(const orbital_data& data, double time, dmath::vec3& planet_origin) const;
                                          
        /*
         * This function computes the trajectories of different particles given their initial
         * motion state and a planetary system object. These predictions are used specifically to
         * render these orbits, so the location predictions are stored as floats. You can specify
         * the starting time of the simulation (which determines the initial position of the
         * planets), the number of steps, the time delta of each step, and the scale of the stored 
         * coordinates. The precision of the predictions depends on the time delta of each step and 
         * the number of steps.
         * 
         * @position_buffers: reference to a vector of vectors of float (does not need to be
         * initialised). Each buffer belongs to a particle, and will contiguously contain the 3D
         * location of the particle at each time step, the components of the coordinates are also
         * contiguous (x1, y1, z1, x2, y2, z2, etc). These buffers can directly be used to render
         * the trajectories of the praticles. Returns a buffer of size (predictor_steps + 1) * 3.
         * @states: the initial motion states of each particle.
         * @config: struct of type fixed_time_trajectory_config (defined above) with the
         * configuration parameters of the trajectory, .
         */
        void computeTrajectoriesRender(std::vector<std::vector<GLfloat>>& position_buffers,
                                       std::vector<struct particle_state>& states,
                                       const struct fixed_time_trajectory_config& config) const;

/*
 * This function computes the approximate trajectories of different particles given their initial
 * motion state and a planetary system object. Stores the positions of each particle at each time
 * as double precision coordinates. You can specify the starting time of the simulation (which
 * determines the initial position of the planets), the number of steps, and the time delta of
 * each step. The precision of the predictions depends on the time delta of each step and the
 * number of steps.
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
/*
void compute_trajectories_double(const PlanetarySystem* planet_system,
                                 std::vector<std::vector<dmath::vec3>>& positions,
                                 std::vector<struct particle_state>& states,
                                 double start_time, uint predictor_delta_t_secs,
                                 uint predictor_steps);

*/

};


#endif