#include <iostream>
#include <memory>

#include <tinyxml2.h>

#include "load_star_system.hpp"
#include "xml_utils.hpp"
#include "../log.hpp"
#include "../Physics.hpp"
#include "../maths_funcs.hpp"
#include "../../assets/PlanetarySystem.hpp"
#include "../../assets/Planet.hpp"


void load_star(const tinyxml2::XMLElement* star_element, struct star& system_star){
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


int load_planets(const tinyxml2::XMLElement* planets_element, planet_map& planets, RenderContext* render_context){
    const tinyxml2::XMLElement* planet_element = planets_element->FirstChildElement("planet");
    const char* name;
    std::pair<planet_map::iterator, bool> res;
    std::hash<std::string> str_hash;

    if(!planet_element)
        return EXIT_FAILURE;

    while(planet_element){
        const tinyxml2::XMLElement* param_element;
        std::unique_ptr<Planet> current_planet(new Planet(render_context));
        orbital_data& data = current_planet->getOrbitalData();

        if(get_attribute(planet_element, "name", &name) == EXIT_FAILURE)
            name = "unk";
        current_planet->setName(name);

        if(get_double(planet_element, "mass", data.mass) == EXIT_FAILURE)
            data.mass = 0.0;
        if(get_double(planet_element, "radius", data.radius) == EXIT_FAILURE)
            data.radius = 0.0;

        param_element = get_element(planet_element, "semi_major_axis");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.semi_major_axis_0) == EXIT_FAILURE)
            data.semi_major_axis_0 = 0.0;
        if(get_double(param_element, "derivative", 
                      data.semi_major_axis_d) == EXIT_FAILURE)
            data.semi_major_axis_d = 0.0;

        param_element = get_element(planet_element, "eccentricity");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.eccentricity_0) == EXIT_FAILURE)
            data.eccentricity_0 = 0.0;
        if(get_double(param_element, "derivative", data.eccentricity_d) == EXIT_FAILURE)
            data.eccentricity_d = 0.0;

        param_element = get_element(planet_element, "inclination");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.inclination_0) == EXIT_FAILURE)
            data.inclination_0 = 0.0;
        if(get_double(param_element, "derivative", data.inclination_d) == EXIT_FAILURE)
            data.inclination_d = 0.0;

        param_element = get_element(planet_element, "mean_longitude");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.mean_longitude_0) == EXIT_FAILURE)
            data.mean_longitude_0 = 0.0;
        if(get_double(param_element, "derivative",
                      data.mean_longitude_d) == EXIT_FAILURE)
            data.mean_longitude_d = 0.0;

        param_element = get_element(planet_element, "longitude_perigee");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.longitude_perigee_0) == EXIT_FAILURE)
            data.longitude_perigee_0 = 0.0;
        if(get_double(param_element, "derivative", 
                      data.longitude_perigee_d) == EXIT_FAILURE)
            data.longitude_perigee_d = 0.0;

        param_element = get_element(planet_element, "long_asc_node");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.long_asc_node_0) == EXIT_FAILURE)
            data.long_asc_node_0 = 0.0;
        if(get_double(param_element, "derivative", data.long_asc_node_d) == EXIT_FAILURE)
            data.long_asc_node_d = 0.0;

        data.inclination_0 *= ONE_DEG_IN_RAD;
        data.inclination_d *= ONE_DEG_IN_RAD;
        data.mean_longitude_0 *= ONE_DEG_IN_RAD;
        data.mean_longitude_d *= ONE_DEG_IN_RAD;
        data.longitude_perigee_0 *= ONE_DEG_IN_RAD;
        data.longitude_perigee_d *= ONE_DEG_IN_RAD;
        data.long_asc_node_0 *= ONE_DEG_IN_RAD;
        data.long_asc_node_d *= ONE_DEG_IN_RAD;
        double mean_anomaly_d = data.mean_longitude_d - data.longitude_perigee_d;
        double period = ((2 * M_PI) / mean_anomaly_d);
        data.period = period;

        current_planet->setID(str_hash(name));
        res = planets.insert({current_planet->getId(), std::move(current_planet)});

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


void load_star_system(PlanetarySystem* system, RenderContext* render_context, const char* path){
    tinyxml2::XMLDocument doc;
    const tinyxml2::XMLElement* system_element,* star_element,* planets_element;
    const char* system_name;
    doc.LoadFile(path);
    struct star system_star;
    std::unique_ptr<planet_map> planets(new planet_map());

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

    if(get_attribute(system_element, "name", &system_name))
        system_name = "unk";

    star_element = get_element(system_element, "star");
    if(!star_element)
        return;

    load_star(star_element, system_star);

    planets_element = get_element(star_element, "planets");
    if(!star_element)
        return;

    if(load_planets(planets_element, *planets.get(), render_context) == EXIT_FAILURE){
        std::cerr << "load_star_system::load_star_system: failed to load star system from"
                  << "file " << path << std::endl;
        log("load_star_system::load_star_system: failed to load star system from file ", path);
    }

    system->setPlanetMap(planets);
    system->setSystemName(system_name);
    system->setStar(system_star);
}

