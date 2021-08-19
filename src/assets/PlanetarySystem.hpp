#ifndef PLANETARY_SYSTEM_HPP
#define PLANETARY_SYSTEM_HPP

#include <unordered_map>
#include <string>
#include <memory>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Planet.hpp"
#include "../core/maths_funcs.hpp"


class RenderContext;

typedef std::unordered_map<std::uint32_t, Planet> planet_map;


// simple star
struct star{
    std::string star_name;
    std::string description;
    double mass;
};


class PlanetarySystem{
    private:
        planet_map m_planets;
        std::string m_system_name;
        struct star m_system_star;

        RenderContext* m_render_context;
    public:
        PlanetarySystem(RenderContext* render_context);
        ~PlanetarySystem();

        planet_map& getPlanets();
        const planet_map& getPlanets() const;
};


#endif