#ifndef LOAD_STAR_SYSTEM_HPP
#define LOAD_STAR_SYSTEM_HPP


#include <unordered_map>
#include <string>


class Resource;

typedef std::unordered_map<std::uint32_t, struct planet> planet_map;


// all of this is just temporal

struct planet{
    std::uint32_t id;
    std::string name;
    double mass, radius, semi_major_axis, semi_major_axis_d, eccentricity, eccentricity_d,
           inclination, inclination_d, mean_longitude, mean_longitude_d, longitude_perigee,
           longitude_perigee_d, long_asc_node, long_asc_node_d;
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


