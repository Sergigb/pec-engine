#include <stb/stb_image.h>

#include "Planet.hpp"
#include "Model.hpp"
#include "RenderContext.hpp"
#include "planet_utils.hpp"
#include "log.hpp"


std::unique_ptr<Model> Planet::m_base32;
std::unique_ptr<Model> Planet::m_base64;
std::unique_ptr<Model> Planet::m_base128;


Planet::Planet(RenderContext* render_context){
    m_surface.max_levels = 6;
    m_surface.planet_sea_level = 6371000.f;
    plutils::build_surface(m_surface);

    m_render_context = render_context;

    m_relative_planet_location = m_render_context->getUniformLocation(SHADER_PLANET, "relative_planet");
    m_texture_scale_location = m_render_context->getUniformLocation(SHADER_PLANET, "texture_scale");
    m_tex_shift_location = m_render_context->getUniformLocation(SHADER_PLANET, "tex_shift");
    m_planet_radius_location = m_render_context->getUniformLocation(SHADER_PLANET, "planet_radius");

    m_planet_texture = m_render_context->getUniformLocation(SHADER_PLANET, "tex");
    m_elevation_texture = m_render_context->getUniformLocation(SHADER_PLANET, "elevation");

    log("Created planet earth");

    // is this needed?
    //glUniform1i(m_planet_texture, TEXTURE_LOCATION);
    //glUniform1i(m_elevation_texture, ELEVATION_LOCATION);
}


Planet::~Planet(){
    for(uint i=0; i < 6; i++){
        plutils::clean_side(m_surface.surface_tree[i], m_surface.max_levels);
    }
}


void Planet::loadBases(const Frustum* frustum, const RenderContext* render_context){
    Planet::m_base32.reset(new Model("../data/base32.dae", nullptr, SHADER_PLANET, frustum, render_context, math::vec3(1.0, 1.0, 1.0)));
    Planet::m_base64.reset(new Model("../data/base64.dae", nullptr, SHADER_PLANET, frustum, render_context, math::vec3(1.0, 1.0, 1.0)));
    //base128.reset(new Model("../data/base128.dae", nullptr, SHADER_PLANET, frustum, render_context, math::vec3(1.0, 1.0, 1.0)));
    Planet::m_base32->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
    Planet::m_base64->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
    //base128->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
}


