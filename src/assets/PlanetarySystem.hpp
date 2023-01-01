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
class AssetManagerInterface;

typedef std::unordered_map<std::uint32_t, std::unique_ptr<Planet>> planet_map;


// simple star
struct star{
    std::string star_name;
    std::string description;
    double mass;
};


class PlanetarySystem{
    private:
        std::unique_ptr<planet_map> m_planets;
        std::string m_system_name;
        struct star m_system_star;

        RenderContext* m_render_context;
        AssetManagerInterface* m_asset_manager;
    public:
        /*
         * Constructor
         *
         * @render_context: raw pointer to the render context.
         */
        PlanetarySystem(RenderContext* render_context);
        ~PlanetarySystem();

        /*
         * Sets the planet map that contains the planets of the system. The system planet map is an
         * unordered map where the values are Planet objects and the keys are the IDs of the 
         * planets. These IDs are created, in theory, by hashing the names of the planets.
         *
         * @map: rvalue reference to a unique pointer containing a planet map.
         */
        void setPlanetMap(std::unique_ptr<planet_map>&& map);

        /*
         * Sets the system name.
         *
         * @name: pointer to a string with the name of the system. The name is copied, the string 
         * should be null terminated.
         */
        void setSystemName(const char* name);

        /*
         * Sets the star of the system. See the definition of the star struct defined above.
         *
         * @sysem_star: reference to an object of type star.
         */
        void setStar(star& system_star);

        /*
         * Calls the orbit render buffer update methods of each individual planet.
         *
         * @current_time: current time in centuries.
         */
        void updateRenderBuffers(const double current_time);

        /*
         * Calls the orbital elements update methods of each individual planet.
         *
         * @current_time: current time in centuries.
         */
        void updateOrbitalElements(const double cent_since_j2000);
        
        /*
         * Calls the update method of each planet that updates their registered kinematics.
         */
        void updateKinematics();

        /*
         * Calls the orbit render method of each one of the planets of the system.
         */
        void renderOrbits() const;

        /*
         * Returns a constant reference to the planets map of the system.
         */
        const planet_map& getPlanets() const;

        /*
         * Returns reference to the planets map of the system.
         */
        planet_map& getPlanets();

        /*
         * Returns a constant reference to the star of the system.
         */
        const star& getStar() const;

        /*
         * Returns a constant reference to the string containing the name of the system.
         */
        const std::string& getSystemName() const;
};


#endif