#include <stb/stb_image.h>
#include <thread>

#include "Planet.hpp"
#include "Model.hpp"
#include "RenderContext.hpp"
#include "planet_utils.hpp"
#include "log.hpp"


std::unique_ptr<Model> Planet::m_base32;
std::unique_ptr<Model> Planet::m_base64;
std::unique_ptr<Model> Planet::m_base128;


#define TEXTURE_LOCATION 0
#define ELEVATION_LOCATION 1


Planet::Planet(RenderContext* render_context){
    m_surface.max_levels = 6;
    m_surface.planet_sea_level = 6371000.f;
    build_surface(m_surface);

    m_render_context = render_context;

    m_relative_planet_location = m_render_context->getUniformLocation(SHADER_PLANET, "relative_planet");
    m_texture_scale_location = m_render_context->getUniformLocation(SHADER_PLANET, "texture_scale");
    m_tex_shift_location = m_render_context->getUniformLocation(SHADER_PLANET, "tex_shift");
    m_planet_radius_location = m_render_context->getUniformLocation(SHADER_PLANET, "planet_radius");

    m_planet_texture = m_render_context->getUniformLocation(SHADER_PLANET, "tex");
    m_elevation_texture = m_render_context->getUniformLocation(SHADER_PLANET, "elevation");

    m_render_context->useProgram(SHADER_PLANET);
    glUniform1i(m_planet_texture, TEXTURE_LOCATION);
    glUniform1i(m_elevation_texture, ELEVATION_LOCATION);

    m_planet_transform = dmath::identity_mat4();
}


Planet::~Planet(){
    for(uint i=0; i < 6; i++){
        clean_side(m_surface.surface_tree[i], m_surface.max_levels);
    }
    // still not checking if we have threads running in the background
    texture_free(m_surface);
}


void Planet::loadBases(const Frustum* frustum, const RenderContext* render_context){
    Planet::m_base32.reset(new Model("../data/base32.dae", nullptr, SHADER_PLANET, frustum, render_context, math::vec3(1.0, 1.0, 1.0)));
    Planet::m_base64.reset(new Model("../data/base64.dae", nullptr, SHADER_PLANET, frustum, render_context, math::vec3(1.0, 1.0, 1.0)));
    //base128.reset(new Model("../data/base128.dae", nullptr, SHADER_PLANET, frustum, render_context, math::vec3(1.0, 1.0, 1.0)));
    Planet::m_base32->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
    Planet::m_base64->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
    //base128->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
}


void Planet::render_side(struct surface_node& node, math::mat4& planet_transform_world, int max_level, const dmath::vec3& cam_origin, double sea_level){
    // PRECOMPUTE THIS VVVVV
    dmath::vec3 path_translation_normd = dmath::vec3(dmath::quat_to_mat4(node.base_rotation) * dmath::vec4(dmath::normalise(node.patch_translation), 1.0)) * sea_level;
    double distance = dmath::distance(path_translation_normd, cam_origin);
    bool texture_is_loaded = true;

    if(node.scale * sea_level * 1.5 > distance && node.level < max_level){
        for(uint i = 0; i < 4; i++){
            render_side(*node.childs[i].get(), planet_transform_world, max_level, cam_origin, sea_level);
        }
        return;
    }

    if(node.level > 4){
        if(node.uppermost_textured_parent->texture_loaded){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, node.uppermost_textured_parent->tex_id);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, node.uppermost_textured_parent->e_tex_id);

            node.uppermost_textured_parent->ticks_since_last_use = 0;  
        }
        else{
            if(!node.uppermost_textured_parent->loading && !node.uppermost_textured_parent->data_ready){
                node.uppermost_textured_parent->loading = true;
                std::thread thread(async_texture_load, node.uppermost_textured_parent, &m_surface);
                thread.detach();
                //async_texture_load(node.uppermost_textured_parent, &m_surface);
                //bind_loaded_texture(*node.uppermost_textured_parent, m_surface);
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, node.tex_id_fl);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, node.e_tex_id_fl);
            texture_is_loaded = false;
        }
    }
    else{
        if(node.texture_loaded){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, node.tex_id);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, node.e_tex_id);

            node.ticks_since_last_use = 0;
        }
        else{
            if(!node.loading && !node.data_ready){
                node.loading = true;
                std::thread thread(async_texture_load, &node, &m_surface);
                thread.detach();
                //async_texture_load(&node, &m_surface);
                //bind_loaded_texture(node, m_surface);
            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, node.tex_id_fl);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, node.e_tex_id_fl);
            texture_is_loaded = false;
        }
    }

    dmath::mat4 dtransform_rotation, dtransform_planet_relative = dmath::identity_mat4();
    math::mat4 transform_planet_relative, scale_transform;
    dtransform_rotation = dmath::quat_to_mat4(node.base_rotation);
    dtransform_planet_relative = dmath::translate(dtransform_planet_relative, node.patch_translation);
    dtransform_planet_relative = dtransform_rotation * dtransform_planet_relative;
    std::copy(dtransform_planet_relative.m, dtransform_planet_relative.m + 16, transform_planet_relative.m);

    scale_transform = math::identity_mat4();
    scale_transform.m[0] = node.scale;
    scale_transform.m[5] = node.scale;
    scale_transform.m[10] = node.scale;
    transform_planet_relative = transform_planet_relative * scale_transform;

    glUniformMatrix4fv(m_relative_planet_location, 1, GL_FALSE, transform_planet_relative.m);

    if(texture_is_loaded){
        glUniform2fv(m_tex_shift_location, 1, node.tex_shift.v);
        glUniform1f(m_texture_scale_location, node.texture_scale);
    }
    else{
        glUniform2fv(m_tex_shift_location, 1, node.tex_shift_lod.v);
        glUniform1f(m_texture_scale_location, node.texture_scale_lod);
    }

    if(node.level >= 1 && node.level < 3){
        Planet::m_base32->render_terrain(planet_transform_world);
    }
    else if(node.level >= 3 && node.level < 7){ // 128x128 disabled
        Planet::m_base64->render_terrain(planet_transform_world);
    }
    else{
        Planet::m_base128->render_terrain(planet_transform_world);
    }
}


void Planet::render(const dmath::vec3& cam_translation){
    render(cam_translation, m_planet_transform);
}


void Planet::render(const dmath::vec3& cam_translation, const dmath::mat4 transform){
    bind_loaded_textures(m_surface);

    math::mat4 planet_transform_world;
    dmath::mat4 dplanet_transform_world = transform;
    dplanet_transform_world.m[12] -= cam_translation.v[0];
    dplanet_transform_world.m[13] -= cam_translation.v[1];
    dplanet_transform_world.m[14] -= cam_translation.v[2];
    std::copy(dplanet_transform_world.m, dplanet_transform_world.m + 16, planet_transform_world.m);     

    m_render_context->useProgram(SHADER_PLANET);
    glUniform1f(m_planet_radius_location, m_surface.planet_sea_level);

    for(uint i=0; i < 6; i++){
        //dmath::vec3 t(6300000.0, 0.0, 0.0);
        render_side(m_surface.surface_tree[i], planet_transform_world, m_surface.max_levels, cam_translation, m_surface.planet_sea_level);
    }

    texture_free(m_surface);
}


dmath::mat4& Planet::getTransform(){
    return m_planet_transform;
}


dmath::mat4 Planet::getTransform() const{
    return m_planet_transform;
}

