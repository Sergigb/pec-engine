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
        PlanetarySystem(RenderContext* render_context);
        ~PlanetarySystem();

        void setPlanetMap(std::unique_ptr<planet_map>& map);
        void setSystemName(const char* name);
        void setStar(star& system_star);

        void updateRenderBuffers(const double current_time);
        void updateOrbitalElements(const double cent_since_j2000);
        void updateKinematics();

        void renderOrbits() const;

        const planet_map& getPlanets() const;
        planet_map& getPlanets();
        const star& getStar() const;
        const std::string& getSystemName() const;
};


#endif