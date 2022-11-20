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
    const char* name, *path;
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
        if(get_attribute(planet_element, "thumbnail", &path) == EXIT_FAILURE)
            path = "../data/planet_thumbnails/Default.png";
        current_planet->loadThumbnail(path);

        if(get_double(planet_element, "mass", data.m) == EXIT_FAILURE)
            data.m = 0.0;
        if(get_double(planet_element, "radius", data.r) == EXIT_FAILURE)
            data.r = 0.0;

        param_element = get_element(planet_element, "semi_major_axis");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.a_0) == EXIT_FAILURE)
            data.a_0 = 0.0;
        if(get_double(param_element, "derivative", 
                      data.a_d) == EXIT_FAILURE)
            data.a_d = 0.0;

        param_element = get_element(planet_element, "eccentricity");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.e_0) == EXIT_FAILURE)
            data.e_0 = 0.0;
        if(get_double(param_element, "derivative", data.e_d) == EXIT_FAILURE)
            data.e_d = 0.0;

        param_element = get_element(planet_element, "inclination");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.i_0) == EXIT_FAILURE)
            data.i_0 = 0.0;
        if(get_double(param_element, "derivative", data.i_d) == EXIT_FAILURE)
            data.i_d = 0.0;

        param_element = get_element(planet_element, "mean_longitude");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.L_0) == EXIT_FAILURE)
            data.L_0 = 0.0;
        if(get_double(param_element, "derivative", data.L_d) == EXIT_FAILURE)
            data.L_d = 0.0;

        param_element = get_element(planet_element, "longitude_perigee");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.p_0) == EXIT_FAILURE)
            data.p_0 = 0.0;
        if(get_double(param_element, "derivative", data.p_d) == EXIT_FAILURE)
            data.p_d = 0.0;

        param_element = get_element(planet_element, "long_asc_node");
        if(!param_element)
            return EXIT_FAILURE;
        if(get_double(param_element, "value", data.W_0) == EXIT_FAILURE)
            data.W_0 = 0.0;
        if(get_double(param_element, "derivative", data.W_d) == EXIT_FAILURE)
            data.W_d = 0.0;

        data.i_0 *= ONE_DEG_IN_RAD;
        data.i_d *= ONE_DEG_IN_RAD;
        data.L_0 *= ONE_DEG_IN_RAD;
        data.L_d *= ONE_DEG_IN_RAD;
        data.p_0 *= ONE_DEG_IN_RAD;
        data.p_d *= ONE_DEG_IN_RAD;
        data.W_0 *= ONE_DEG_IN_RAD;
        data.W_d *= ONE_DEG_IN_RAD;
        // avoids valgrind reporting uninitialised values
        data.pos = dmath::vec3(0.0, 0.0, 0.0);
        data.pos_prev = dmath::vec3(0.0, 0.0, 0.0);
        data.v = 0.0;
        data.p = data.p_0 + data.p_d * 0.0;
        double mean_anomaly_d = data.L_d - data.p;

        data.period = ((2 * M_PI) / mean_anomaly_d);

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

