#include "predictors.hpp"



inline dmath::vec3 computePlanetPosition(const orbital_data& data, double time){
    dmath::vec3 planet_origin;
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

    return planet_origin;
}


