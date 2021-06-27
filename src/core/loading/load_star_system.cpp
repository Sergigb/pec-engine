#include <iostream>

#include <tinyxml2.h>

#include "load_star_system.hpp"
#include "xml_utils.hpp"
#include "../log.hpp"
#include "../Physics.hpp"
#include "../maths_funcs.hpp"


void load_star(const tinyxml2::XMLElement* star_element, star& system_star){
    const char* star_name,* description;
    double mass;

    if(get_attribute(star_element, "name", &star_name) == EXIT_FAILURE)
        star_name = "unk";
    if(get_double(star_element, "mass", mass) == EXIT_FAILURE)
        mass = 0.0;
    if(get_string(star_element, "description", &description) == EXIT_FAILURE)
        description = "";

    system_star.star_name = star_name;
    system_star.mass = mass;
    system_star.description = description;
}


int load_planets(const tinyxml2::XMLElement* planets_element, planet_map& planets){
    const tinyxml2::XMLElement* planet_element = planets_element->FirstChildElement("planet");
    const char* name;
    std::pair<planet_map::iterator, bool> res;
    std::hash<std::string> str_hash;

    if(!planet_element)
        return EXIT_FAILURE;

    while(planet_element){
        const tinyxml2::XMLElement* param_element;
        planet current_planet;

        if(get_attribute(planet_element, "name", &name) == EXIT_FAILURE)
            name = "unk";
        current_planet.name = name;

        if(get_double(planet_element, "mass", current_planet.mass) == EXIT_FAILURE)
            current_planet.mass = 0.0;
        if(get_double(planet_element, "radius", current_planet.radius) == EXIT_FAILURE)
            current_planet.radius = 0.0;

        param_element = get_element(planet_element, "semi_major_axis");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", current_planet.semi_major_axis_0) == EXIT_FAILURE)
            current_planet.semi_major_axis_0 = 0.0;
        if(get_double(param_element, "derivative", 
                      current_planet.semi_major_axis_d) == EXIT_FAILURE)
            current_planet.semi_major_axis_d = 0.0;

        param_element = get_element(planet_element, "eccentricity");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", current_planet.eccentricity_0) == EXIT_FAILURE)
            current_planet.eccentricity_0 = 0.0;
        if(get_double(param_element, "derivative", current_planet.eccentricity_d) == EXIT_FAILURE)
            current_planet.eccentricity_d = 0.0;

        param_element = get_element(planet_element, "inclination");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", current_planet.inclination_0) == EXIT_FAILURE)
            current_planet.inclination_0 = 0.0;
        if(get_double(param_element, "derivative", current_planet.inclination_d) == EXIT_FAILURE)
            current_planet.inclination_d = 0.0;

        param_element = get_element(planet_element, "mean_longitude");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", current_planet.mean_longitude_0) == EXIT_FAILURE)
            current_planet.mean_longitude_0 = 0.0;
        if(get_double(param_element, "derivative",
                      current_planet.mean_longitude_d) == EXIT_FAILURE)
            current_planet.mean_longitude_d = 0.0;

        param_element = get_element(planet_element, "longitude_perigee");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", current_planet.longitude_perigee_0) == EXIT_FAILURE)
            current_planet.longitude_perigee_0 = 0.0;
        if(get_double(param_element, "derivative", 
                      current_planet.longitude_perigee_d) == EXIT_FAILURE)
            current_planet.longitude_perigee_d = 0.0;

        param_element = get_element(planet_element, "long_asc_node");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", current_planet.long_asc_node_0) == EXIT_FAILURE)
            current_planet.long_asc_node_0 = 0.0;
        if(get_double(param_element, "derivative", current_planet.long_asc_node_d) == EXIT_FAILURE)
            current_planet.long_asc_node_d = 0.0;

        current_planet.inclination_0 *= ONE_DEG_IN_RAD;
        current_planet.inclination_d *= ONE_DEG_IN_RAD;
        current_planet.mean_longitude_0 *= ONE_DEG_IN_RAD;
        current_planet.mean_longitude_d *= ONE_DEG_IN_RAD;
        current_planet.longitude_perigee_0 *= ONE_DEG_IN_RAD;
        current_planet.longitude_perigee_d *= ONE_DEG_IN_RAD;
        current_planet.long_asc_node_0 *= ONE_DEG_IN_RAD;
        current_planet.long_asc_node_d *= ONE_DEG_IN_RAD;
        double mean_anomaly_d = current_planet.mean_longitude_d - current_planet.longitude_perigee_d;
        double period = ((2 * M_PI) / mean_anomaly_d);
        current_planet.period = period;

        current_planet.id = str_hash(name);
        res = planets.insert({current_planet.id, current_planet});

        if(!res.second){
            log("load_star_system::load_planets: failed to insert planet with id ", 
                (std::uint32_t)str_hash(name), " (collided with another resource with"
                " the same name)");

            std::cerr << "load_star_system::load_planets: failed to insert planet with id " 
                      << (std::uint32_t)str_hash(name) << " (collided with another "
                      "resource with same name)" << std::endl;
        }

        planet_element = planet_element->NextSiblingElement("planet");
    }

    return EXIT_SUCCESS;
}


void load_star_system(struct planetary_system& system, const char* path){
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLElement* system_element,* star_element,* planets_element;
    const char* system_name;
    doc.LoadFile(path);

    if(doc.Error()){
        std::cerr << "load_star_system::load_star_system: LoadFile returned an error with code " 
                  << doc.ErrorID() << " for document " << path << std::endl;
        log("load_resources::load_resources: LoadFile returned an error with code ",
            doc.ErrorID(), " for document ", path);
        return;
    }

    system_element = doc.RootElement();

    if(!system_element){
        std::cerr << "load_star_system::load_star_system: can't get the root element for the "
                  "document " << path << std::endl;
        log("load_star_system::load_star_system: can't get the root element for the document ",
            path);
        return;
    }

    if(get_attribute(system_element, "name", &system_name)){
        std::cerr << "load_star_system::load_star_system: planetary system name is missing in "
                     "file " << path << std::endl;
        log("load_star_system::load_star_system: planetary system name is missing in file ", path);
        system_name = "unk";
    }
    system.system_name = system_name;

    star_element = get_element(system_element, "star");
    if(!star_element)
        return;

    load_star(star_element, system.system_star);

    planets_element = get_element(star_element, "planets");
    if(!star_element)
        return;

    if(load_planets(planets_element, system.planets) == EXIT_FAILURE){
        std::cerr << "load_star_system::load_star_system: failed to load star system from"
                  << "file " << path << std::endl;
        log("load_star_system::load_star_system: failed to load star system from file ", path);
    }
}

