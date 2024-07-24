#include "Predictor.hpp"
#include "BaseApp.hpp"
#include "Physics.hpp"
#include "AssetManager.hpp"
#include "solvers.hpp"
#include "../assets/PlanetarySystem.hpp"



Predictor::Predictor(const BaseApp* app){
    m_app = app;
}


Predictor::~Predictor(){

}


void Predictor::computeObjectPosVel(const orbital_data& data, std::uint32_t body_target, 
                                    double time, bool match_frame, dmath::vec3& origin,
                                    dmath::vec3& velocity) const{
    const PlanetarySystem* planet_system = m_app->getAssetManager()->m_planetary_system.get();
    double time_cent = time / SECONDS_IN_A_CENTURY;
    double a = data.a_0 + data.a_d * time_cent;
    double e = data.e_0 + data.e_d * time_cent;
    double inc = data.i_0 + data.i_d * time_cent;
    double L = data.L_0 + data.L_d * time_cent;
    double p = data.p_0 + data.p_d * time_cent;
    double W = data.W_0 + data.W_d * time_cent;

    double M = L - p;
    double w = p - W;
    double E = M;
    double ecc_d = 10.;

    a *= AU_TO_METERS;

    int iter = 0;
    while(std::abs(ecc_d) > 1e-6 && iter < MAX_CONIC_SOLVER_ITER){
        ecc_d = (E - e * std::sin(E) - M) / (1 - e * std::cos(E));
        E -= ecc_d;
        iter++;
    }

    double v = 2 * std::atan(std::sqrt((1 + e) / (1 - e)) * std::tan(E / 2));

    double rad = a * (1 - e * std::cos(E));

    origin.v[0] = rad * (std::cos(W) * std::cos(w + v) -
                  std::sin(W) * std::sin(w + v) * std::cos(inc));
    origin.v[1] = rad * (std::sin(inc) * std::sin(w + v));
    origin.v[2] = rad * (std::sin(W) * std::cos(w + v) +
                  std::cos(W) * std::sin(w + v) *std::cos(inc));

    double mu;
    if(body_target == 0)
        mu = GRAVITATIONAL_CONSTANT * planet_system->getStar().mass;
    else
        mu = GRAVITATIONAL_CONSTANT * 
             planet_system->getPlanets().at(body_target)->getOrbitalData().m;

    double h = std::sqrt(mu * a * (1 - (e * e)));
    double p_ = a * (1 - (e * e));

    velocity.v[0] = (origin.v[0] * h * e / (rad * p_)) * std::sin(v) - (h / rad) *
                    (std::cos(W) * std::sin(w + v) + std::sin(W) *
                    std::cos(w + v) * std::cos(inc));
    velocity.v[1] = (origin.v[1] * h * e / (rad * p_)) * std::sin(v) + (h / rad) * 
                    (std::cos(w + v) * std::sin(inc));
    velocity.v[2] = (origin.v[2] * h * e / (rad * p_)) * std::sin(v) - (h / rad) * 
                    (std::sin(W) * std::sin(w + v) - std::cos(W) * 
                    std::cos(w + v) * std::cos(inc));

    if(match_frame && body_target != 0){
        dmath::vec3 target_pos, target_vel;
        const struct orbital_data& target_data = 
            planet_system->getPlanets().at(body_target)->getOrbitalData();

        computeObjectPosVel(target_data, 0, time, false, target_pos, target_vel);
        origin += target_pos;
        velocity += target_vel;
    }
}


void Predictor::computeObjectPos(const orbital_data& data, double time,
                                 dmath::vec3& planet_origin) const{
    double e, W, w, inc, a, L, p, M, v;

    // update planet position
    a = data.a_0 + data.a_d * time;
    e = data.e_0 + data.e_d * time;
    inc = data.i_0 + data.i_d * time;
    L = data.L_0 + data.L_d * time;
    p = data.p_0 + data.p_d * time;
    W = data.W_0 + data.W_d * time;


    M = L - p;
    w = p - W;

    double E = M;
    double ecc_d = 10.;
    int iter = 0;
    while(std::abs(ecc_d) > 1e-6 && iter < MAX_CONIC_SOLVER_ITER){
        ecc_d = (E - e * std::sin(E) - M) / (1 - e * std::cos(E));
        E -= ecc_d;
        iter++;
    }

    v = 2 * std::atan(std::sqrt((1 + e) / (1 - e)) * std::tan(E / 2));

    double rad = a * (1 - e * std::cos(E)) * AU_TO_METERS;

    planet_origin.v[0] = rad * (std::cos(W) * std::cos(w + v) -
                         std::sin(W) * std::sin(w + v) * std::cos(inc));
    planet_origin.v[1] = rad * (std::sin(inc) * std::sin(w + v));
    planet_origin.v[2] = rad * (std::sin(W) * std::cos(w + v) +
                         std::cos(W) * std::sin(w + v) *std::cos(inc));
}


void Predictor::computeTrajectoriesRender(std::vector<std::vector<GLfloat>>& position_buffers,
                                         std::vector<struct particle_state>& states,
                                         const struct fixed_time_trajectory_config& config) const{
    assert(states.size());
    assert(config.predictor_delta_t_secs);
    assert(config.predictor_steps);
    const PlanetarySystem* planet_system = m_app->getAssetManager()->m_planetary_system.get();

    planet_map::const_iterator it;
    const planet_map& planets = planet_system->getPlanets();
    double star_mass = planet_system->getStar().mass;

    double time = config.predictor_start_time / SECONDS_IN_A_CENTURY;
    double predictor_delta_t_cent = config.predictor_delta_t_secs / SECONDS_IN_A_CENTURY;

    // reserve buffer memory to their expected size to avoid dynamic re-allocation by std
    position_buffers.clear();
    position_buffers.reserve(states.size());
    for(uint i=0; i < states.size(); i++){
        position_buffers.push_back(std::vector<GLfloat>());
        position_buffers.at(i).reserve(3 * config.predictor_steps);

        position_buffers.at(i).push_back(states.at(i).origin.v[0] / config.predictor_scale);
        position_buffers.at(i).push_back(states.at(i).origin.v[1] / config.predictor_scale);
        position_buffers.at(i).push_back(states.at(i).origin.v[2] / config.predictor_scale);
    }

    for(int i=0; i < config.predictor_steps; i++){
        time += predictor_delta_t_cent;
        // iterate over each planet
        for(it=planets.begin();it!=planets.end();it++){
            const orbital_data& data = it->second->getOrbitalData();
            dmath::vec3 planet_origin;
            computeObjectPos(data, time, planet_origin);

            // iterate over each object
            for(uint j = 0; j < states.size(); j++){
                struct particle_state& current = states.at(j);

                // update gravity force on current particle
                double Rh = dmath::distance(planet_origin, current.origin);
                double acceleration = GRAVITATIONAL_CONSTANT * (data.m / (Rh*Rh));
                dmath::vec3 f = dmath::normalise(planet_origin - current.origin)
                                                 * acceleration * current.mass;
                current.total_force += f;
            }
        }

        for(uint j = 0; j < states.size(); j++){
            struct particle_state& current = states.at(j);
            // force of the star
            double Rh = dmath::length(current.origin); // centered star com at (0, 0, 0)
            double acceleration = GRAVITATIONAL_CONSTANT * (star_mass / (Rh*Rh));
            //dmath::vec3 f = dmath::normalise(-current.origin) * acceleration * current.mass; // does not work?
            dmath::vec3 f = dmath::normalise(dmath::vec3(-current.origin.v[0],
                                                         -current.origin.v[1],
                                                         -current.origin.v[2]))
                                             * acceleration * current.mass;

            current.total_force += f;

            // solve motion current step
            solverSymplecticEuler(current, config.predictor_delta_t_secs);

            // udpate buffers
            position_buffers.at(j).push_back(current.origin.v[0] / config.predictor_scale);
            position_buffers.at(j).push_back(current.origin.v[1] / config.predictor_scale);
            position_buffers.at(j).push_back(current.origin.v[2] / config.predictor_scale);
        }
    }
}


/*void compute_trajectories_double(const PlanetarySystem* planet_system,
                                 std::vector<std::vector<dmath::vec3>>& positions,
                                 std::vector<struct particle_state>& states,
                                 double start_time, uint predictor_delta_t_secs,
                                 uint predictor_steps){
    assert(planet_system);
    assert(states.size());
    assert(predictor_delta_t_secs);
    assert(predictor_steps);

    planet_map::const_iterator it;
    const planet_map& planets = planet_system->getPlanets();
    double star_mass = planet_system->getStar().mass;

    double time = start_time / SECONDS_IN_A_CENTURY;
    double predictor_delta_t_cent = predictor_delta_t_secs / SECONDS_IN_A_CENTURY;

    // reserve buffer memory to their expected size to avoid dynamic re-allocation by std
    positions.clear();
    positions.reserve(states.size());
    for(uint i=0; i < states.size(); i++){
        positions.push_back(std::vector<dmath::vec3>()); 
        positions.at(i).reserve(predictor_steps * 3);
    }

    for(uint i=0; i < predictor_steps; i++){
        // iterate over each planet
        for(it=planets.begin();it!=planets.end();it++){
            const orbital_data& data = it->second->getOrbitalData();
            dmath::vec3 planet_origin;
            compute_object_position(data, time, planet_origin);

            // iterate over each object
            for(uint j = 0; j < states.size(); j++){
                struct particle_state& current = states.at(j);

                // update gravity force on current particle
                double Rh = dmath::distance(planet_origin, current.origin);
                double acceleration = GRAVITATIONAL_CONSTANT * (data.m / (Rh*Rh));
                dmath::vec3 f = dmath::normalise(planet_origin - current.origin)
                                                 * acceleration * current.mass;
                current.total_force += f;
            }
        }

        for(uint j = 0; j < states.size(); j++){
            struct particle_state& current = states.at(j);
            // force of the star
            double Rh = dmath::length(current.origin); // centered star com at (0, 0, 0)
            double acceleration = GRAVITATIONAL_CONSTANT * (star_mass / (Rh*Rh));
            dmath::vec3 f = dmath::normalise(-current.origin) * acceleration * current.mass;
            current.total_force += f;

            // solve motion current step
            solverSymplecticEuler(current, predictor_delta_t_secs);

            // append position
            positions.at(j).push_back(current.origin);
        }

        // udpate buffers
        time += predictor_delta_t_cent;
    }
}
*/