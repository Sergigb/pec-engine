#ifndef LOAD_STAR_SYSTEM_HPP
#define LOAD_STAR_SYSTEM_HPP


#include <unordered_map>
#include <string>

#include "../maths_funcs.hpp"


class Resource;

typedef std::unordered_map<std::uint32_t, struct planet> planet_map;


// all of this is just temporal

struct planet{
    std::uint32_t id;
    std::string name;
    // initially read from file
    double mass, radius, semi_major_axis_0, semi_major_axis_d, eccentricity_0, eccentricity_d,
           inclination_0, inclination_d, mean_longitude_0, mean_longitude_d, longitude_perigee_0,
           longitude_perigee_d, long_asc_node_0, long_asc_node_d;
    // updated at each time step
    double semi_major_axis, eccentricity, inclination, mean_longitude, longitude_perigee, long_asc_node;
    // derivated
    double mean_anomaly, arg_periapsis, eccentric_anomaly, std_grav_parameter;

    dmath::vec3 pos, pos_prev, test, test_prev;
};

struct star{
    std::string star_name;
    double mass;
};

struct planetary_system{
    planet_map planets;
    std::string system_name;
    struct star system_star;
};


void load_star_system(struct planetary_system& system, const char* path="../data/star_system.xml");

#endif


