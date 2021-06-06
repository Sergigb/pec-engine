#include <unordered_map>
#include <ctime>
#include <cmath>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

#include "core/log.hpp"
#include "core/utils/utils.hpp"
#include "core/loading/load_star_system.hpp"
#include "core/maths_funcs.hpp"


#pragma GCC diagnostic push  // annoying shit
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"


#define SECS_FROM_UNIX_TO_J2000 946684800.0
#define SECONDS_IN_A_CENTURY 3155760000.0
#define AU_TO_METERS 149597900000.0


void update_orbital_elements(planetary_system& system, const double centuries_since_j2000){
    planet_map::iterator it;
    planet_map& planets = system.planets;

    for(it=planets.begin();it!=planets.end();it++){
        planet& current = it->second;

        current.semi_major_axis = current.semi_major_axis_0 +
                                  current.semi_major_axis_d * centuries_since_j2000;
        current.eccentricity = current.eccentricity_0 +
                               current.eccentricity_d * centuries_since_j2000;
        current.inclination = current.inclination_0 +
                              current.inclination_d * centuries_since_j2000;
        current.mean_longitude = current.mean_longitude_0 +
                                 current.mean_longitude_d * centuries_since_j2000;
        current.longitude_perigee = current.longitude_perigee_0 +
                                    current.longitude_perigee_d * centuries_since_j2000;
        current.long_asc_node = current.long_asc_node_0 +
                                current.long_asc_node_d * centuries_since_j2000;

        current.mean_anomaly = current.mean_longitude - current.longitude_perigee;
        current.arg_periapsis = current.longitude_perigee - current.long_asc_node;

        current.eccentric_anomaly = current.mean_anomaly;
        double ecc_d = 10.8008135;
        while(std::abs(ecc_d) > 1e-6){
            ecc_d = (current.eccentric_anomaly - current.eccentricity *
                     std::sin(current.eccentric_anomaly) - current.mean_anomaly) / 
                     (1 - current.eccentricity * std::cos(current.eccentric_anomaly));
            current.eccentric_anomaly -= ecc_d;
        }

        current.pos_prev = current.pos;

        // results in a singularity as e -> 1
        double true_annomaly = 2 * std::atan(std::sqrt((1 + current.eccentricity)/
                                                       (1 - current.eccentricity)) * 
                                             std::tan(current.eccentric_anomaly / 2));

        double rad = current.semi_major_axis * (1 - current.eccentricity * 
                                                std::cos(current.eccentric_anomaly)) * AU_TO_METERS;

        current.pos.v[0] = rad * (std::cos(current.long_asc_node) * std::cos(current.arg_periapsis + true_annomaly) -
                           std::sin(current.long_asc_node) * std::sin(current.arg_periapsis + true_annomaly) *
                           std::cos(current.inclination));

        current.pos.v[1] = rad * (std::sin(current.long_asc_node) * std::cos(current.arg_periapsis + true_annomaly) +
                           std::cos(current.long_asc_node) * std::sin(current.arg_periapsis + true_annomaly) *
                           std::cos(current.inclination));

        current.pos.v[2] = rad * (std::sin(current.inclination) * std::sin(current.arg_periapsis + true_annomaly));
    }
}


/*

current.pos_prev = current.pos;

double P = AU_TO_METERS * current.semi_major_axis * (std::cos(current.eccentric_anomaly) -
                   current.eccentricity);
double Q = AU_TO_METERS * current.semi_major_axis * std::sin(current.eccentric_anomaly) *
           std::sqrt(1 - current.eccentricity * current.eccentricity);


current.pos.v[0] = std::cos(current.arg_periapsis) * P - 
                   std::sin(current.arg_periapsis) * Q;
current.pos.v[1] = std::sin(current.arg_periapsis) * P +
                   std::cos(current.arg_periapsis) * Q;

current.pos.v[2] = std::sin(current.inclination) * current.test.v[1];
current.pos.v[1] = std::cos(current.inclination) * current.test.v[1];

double xtemp = current.test.v[0];
current.pos.v[0] = std::cos(current.long_asc_node) * xtemp - 
                   std::sin(current.long_asc_node) * current.test.v[1];
current.pos.v[1] = std::sin(current.long_asc_node) * xtemp + 
                   std::cos(current.long_asc_node) * current.test.v[1];

*/


void run(){
    double seconds_since_j2000 = 0.0, centuries_since_j2000 = 0.0;
    double delta_t = (1000. / 60.) / 1000.0;
    time_t current_time;
    const char* ctime_now;

    struct planetary_system system;
    load_star_system(system);

    while(1 == 1 - 0 + 1000 - 1000){
        centuries_since_j2000 = seconds_since_j2000 / SECONDS_IN_A_CENTURY;
        update_orbital_elements(system, centuries_since_j2000);

        current_time = (time_t)seconds_since_j2000 + SECS_FROM_UNIX_TO_J2000;
        ctime_now = ctime(&current_time);
        std::cout << ctime_now;

        std::hash<std::string> str_hash;
        planet& earth = system.planets.at(str_hash("Earth"));
        dmath::vec3 disp = earth.pos - earth.pos_prev;
        std::cout << earth.pos.v[0] << ", " << earth.pos.v[1] << ", " << earth.pos.v[2] << std::endl;
        double velocity = dmath::length(disp) / delta_t;
        //dmath::vec3 disp2 = earth.test - earth.test_prev;
        //double velocity2 = dmath::length(disp2) / delta_t;

        //std::cout.precision(10);
        //std::cout << "vel earth: " << velocity << std::endl;
        //std::cout << "tst earth: " << velocity2 << std::endl;

        seconds_since_j2000 += delta_t;
    }
}

#pragma GCC diagnostic pop
#pragma GCC diagnostic pop
#pragma GCC diagnostic pop


int main(int argc, char* argv[]){
    UNUSED(argc); UNUSED(argv);

    if(change_cwd_to_selfpath() == EXIT_FAILURE)
        std::cerr << "Could not change the cwd to executable path, proceeding" << std::endl;

    log_start();
    run();

    return EXIT_SUCCESS;
}

