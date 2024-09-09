#include <thread>
#include <sstream>

#include <stb/stb_image.h>

#include "PlanetTree.hpp"
#include "Planet.hpp"
#include "Model.hpp"
#include "../core/RenderContext.hpp"
#include "../core/log.hpp"
#include "../core/utils/gl_utils.hpp"


std::unique_ptr<Model> PlanetTree::m_base32;
std::unique_ptr<Model> PlanetTree::m_base64;
std::unique_ptr<Model> PlanetTree::m_base128;

#define ASYNC_PLANET_TEXTURE_LOAD 
#define TEXTURE_LOCATION 0
#define ELEVATION_LOCATION 1


PlanetTree::PlanetTree(){
    m_surface.is_built = false;
}


bool warning_async_notified = false;
PlanetTree::PlanetTree(RenderContext* render_context, Planet* planet){
    m_surface.max_levels = 6;
    m_surface.planet_sea_level = 6371000.f;

    m_surface.is_built = false;

    m_render_context = render_context;
    m_planet = planet;

    m_relative_planet_location = m_render_context->getUniformLocation(SHADER_PLANET, "relative_planet");
    m_texture_scale_location = m_render_context->getUniformLocation(SHADER_PLANET, "texture_scale");
    m_tex_shift_location = m_render_context->getUniformLocation(SHADER_PLANET, "tex_shift");
    m_planet_radius_location = m_render_context->getUniformLocation(SHADER_PLANET, "planet_radius");

    m_planet_texture = m_render_context->getUniformLocation(SHADER_PLANET, "tex");
    m_elevation_texture = m_render_context->getUniformLocation(SHADER_PLANET, "elevation");

    if(!warning_async_notified){
#ifndef ASYNC_PLANET_TEXTURE_LOAD
        std::cout << "PlanetTree::PlanetTree: Asynchronous planet texture loading is not enabled" << std::endl;
        log("PlanetTree::PlanetTree: Asynchronous planet texture loading is not enabled");
#else
        std::cout << "PlanetTree::PlanetTree: Asynchronous planet texture loading is enabled" << std::endl;
        log("PlanetTree::PlanetTree: Asynchronous planet texture loading is enabled");
#endif
        warning_async_notified = true;
    }

    check_gl_errors(true, "PlanetTree::PlanetTree");
}


PlanetTree::~PlanetTree(){
    if(m_surface.is_built){
        //std::cout << "planet tree destructor disabled" << std::endl;
        for(uint i=0; i < 6; i++){
            cleanSide(m_surface.surface_tree[i], m_surface.max_levels);
        }
        // still not checking if we have threads running in the background
        textureFree();
    }
    m_surface.is_built = false;

    check_gl_errors(true, "PlanetTree::~PlanetTree");
}



void PlanetTree::setTransform(struct surface_node& node, const struct surface_node& parent, int sign_side_1, int sign_side_2){
    node.patch_translation = parent.patch_translation;
    node.patch_translation.v[1] += (node.scale / 2) * sign_side_1;
    node.patch_translation.v[2] += (node.scale / 2) * sign_side_2;

    short scale = pow(2.0, node.level);
    node.tex_shift_lod.v[0] = parent.tex_shift_lod.v[0] + (1.0f / scale) * sign_side_1;
    node.tex_shift_lod.v[1] = parent.tex_shift_lod.v[1] + (1.0f / scale) * sign_side_2;

    if(node.level <= 4){
        node.tex_shift.v[0] = 0.0f;
        node.tex_shift.v[1] = 0.0f;
    }
    else{
        scale = pow(2.0, short(node.level - node.uppermost_textured_parent->level) + 1);

        node.tex_shift.v[0] = parent.tex_shift.v[0] + (1.0 / scale) * sign_side_1;
        node.tex_shift.v[1] = parent.tex_shift.v[1] + (1.0 / scale) * sign_side_2;
    }
}


void PlanetTree::bindTexture(struct surface_node& node){
    int tex_x, tex_y, n_channels;
    std::ostringstream fname;

    glGenTextures(1, &node.tex_id);
    glBindTexture(GL_TEXTURE_2D, node.tex_id);
    fname << "../data/earth_textures/"
          << node.level << "_" 
          << (short)node.side << "_" 
          << node.x << "_"
          << node.y << ".png";
    node.texture_loaded = true;

    unsigned char* data = stbi_load(fname.str().c_str(), &tex_x, &tex_y, &n_channels, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_x, tex_y, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    stbi_image_free(data);

    check_gl_errors(true, "PlanetTree::bindTexture");
}


void PlanetTree::bindElevationTexture(struct surface_node& node){
    int tex_x, tex_y, n_channels;
    std::ostringstream fname;

    glGenTextures(1, &node.e_tex_id);
    glBindTexture(GL_TEXTURE_2D, node.e_tex_id);
    fname << "../data/earth_textures/elevation/e_"
          << node.level << "_" 
          << (short)node.side << "_" 
          << node.x << "_"
          << node.y << ".png";

    unsigned char* data = stbi_load(fname.str().c_str(), &tex_x, &tex_y, &n_channels, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_x, tex_y, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    stbi_image_free(data);

    check_gl_errors(true, "PlanetTree::bindElevationTexture");
}


void PlanetTree::buildChilds(struct surface_node& node, int num_levels){

    for(uint i=0; i < 4; i++){
        node.childs[i].reset(new struct surface_node);
    }

    node.childs[0]->x = 2 * node.x;
    node.childs[0]->y = 2 * node.y;

    node.childs[1]->x = 2 * node.x + 1;
    node.childs[1]->y = 2 * node.y;

    node.childs[2]->x = 2 * node.x;
    node.childs[2]->y = 2 * node.y + 1;

    node.childs[3]->x = 2 * node.x + 1;
    node.childs[3]->y = 2 * node.y + 1;


    for(uint i=0; i < 4; i++){
        node.childs[i]->scale = node.scale / 2.0;
        node.childs[i]->base_rotation = node.base_rotation;
        node.childs[i]->level = node.level + 1;
        node.childs[i]->side = node.side;
        node.childs[i]->has_texture = true;
        node.childs[i]->texture_loaded = false;
        node.childs[i]->loading = false;
        node.childs[i]->ticks_since_last_use = 0;
        node.childs[i]->data_ready = false;
        node.childs[i]->has_elevation = true;
        node.childs[i]->tex_id_fl = node.tex_id_fl;
        node.childs[i]->e_tex_id_fl = node.e_tex_id_fl;
        node.childs[i]->texture_scale_lod = node.texture_scale_lod / 2.0;
        if(node.level + 1 < 5){
            node.childs[i]->texture_scale = 1.0;
            node.childs[i]->uppermost_textured_parent = node.childs[i].get();
        }
        else {
            node.childs[i]->texture_scale = node.texture_scale / 2.0;
            node.childs[i]->uppermost_textured_parent = node.uppermost_textured_parent;
        }
    }

    setTransform(*node.childs[0].get(), node, 1, 1);
    setTransform(*node.childs[1].get(), node, -1, 1);
    setTransform(*node.childs[2].get(), node, 1, -1);
    setTransform(*node.childs[3].get(), node, -1, -1);

    if(node.level + 2 <= num_levels){ // node.level + 2 ----> build the childs of the childs you have just built... confusing
        for(uint i = 0; i < 4; i++){
            buildChilds(*node.childs[i].get(), num_levels);
        }
    }
}


void PlanetTree::bindLoadedTexture(struct surface_node& node){
    glGenTextures(1, &node.tex_id);
    glBindTexture(GL_TEXTURE_2D, node.tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, node.tex_x, node.tex_y, 0, GL_RGB, GL_UNSIGNED_BYTE, node.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //std::cout << "Texture bound: " << std::endl;

    stbi_image_free(node.data);
    //free(node.data);

    glGenTextures(1, &node.e_tex_id);
    glBindTexture(GL_TEXTURE_2D, node.e_tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, node.e_tex_x, node.e_tex_y, 0, GL_RED, GL_UNSIGNED_BYTE, node.data_elevation);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    stbi_image_free(node.data_elevation);
    //free(node.data_elevation);

    m_surface.bound_nodes.push_back(&node);
    node.ticks_since_last_use = 0;
    node.texture_loaded = true;
    node.data_ready = false;

    check_gl_errors(true, "PlanetTree::bindLoadedTexture");
}


void PlanetTree::buildSurface(){
    short num_levels = m_surface.max_levels;
    m_surface.is_built = true;

    m_surface.surface_tree[0].side = SIDE_PX;
    m_surface.surface_tree[1].side = SIDE_NX;
    m_surface.surface_tree[2].side = SIDE_PY;
    m_surface.surface_tree[3].side = SIDE_NY;
    m_surface.surface_tree[4].side = SIDE_PZ;
    m_surface.surface_tree[5].side = SIDE_NZ;
    
    m_surface.surface_tree[0].base_rotation = dmath::quat_from_axis_rad(0.0, 1.0, 0.0, 0.0);
    m_surface.surface_tree[1].base_rotation = dmath::quat_from_axis_rad(M_PI, 0.0, 1.0, 0.0);
    m_surface.surface_tree[2].base_rotation = dmath::quat_from_axis_rad(M_PI/2, 0.0, 0.0, 1.0);
    m_surface.surface_tree[3].base_rotation = dmath::quat_from_axis_rad(-M_PI/2, 0.0, 0.0, 1.0);
    m_surface.surface_tree[4].base_rotation = dmath::quat_from_axis_rad(M_PI/2, 0.0, 1.0, 0.0);
    m_surface.surface_tree[5].base_rotation = dmath::quat_from_axis_rad(-M_PI/2, 0.0, 1.0, 0.0);

    for(uint i=0; i < 6; i++){
        m_surface.surface_tree[i].patch_translation = dmath::vec3(0.5, 0.0, 0.0);
        m_surface.surface_tree[i].tex_shift = math::vec2(0.0, 0.0);
        m_surface.surface_tree[i].tex_shift_lod = math::vec2(0.0, 0.0);
        m_surface.surface_tree[i].scale = 1.0;
        m_surface.surface_tree[i].level = 1;
        m_surface.surface_tree[i].has_texture = true;
        m_surface.surface_tree[i].x = 0;
        m_surface.surface_tree[i].y = 0;
        m_surface.surface_tree[i].loading = false;
        m_surface.surface_tree[i].ticks_since_last_use = 0;
        m_surface.surface_tree[i].data_ready = false;
        m_surface.surface_tree[i].has_elevation = true;
        m_surface.surface_tree[i].texture_scale = 1.0f;
        m_surface.surface_tree[i].texture_scale_lod = 1.0f;
        m_surface.surface_tree[i].uppermost_textured_parent = &m_surface.surface_tree[i];
        bindTexture(m_surface.surface_tree[i]);
        m_surface.surface_tree[i].tex_id_fl = m_surface.surface_tree[i].tex_id;
        bindElevationTexture(m_surface.surface_tree[i]);
        m_surface.surface_tree[i].e_tex_id_fl = m_surface.surface_tree[i].e_tex_id;
        //UNUSED(num_levels);
        buildChilds(m_surface.surface_tree[i], num_levels);
    }
}


void PlanetTree::cleanSide(struct surface_node& node, int num_levels){ // num_levels should be a macro in the future
    /*
    Warning: we are not checking if there's any thread loading textures in the background, this should be done in the future!
    */
    if(node.texture_loaded){
        glDeleteTextures(1, &node.tex_id);
        glDeleteTextures(1, &node.e_tex_id);
    }
    if(node.data_ready){
        stbi_image_free(node.data);
        stbi_image_free(node.data_elevation);
    }
    if(node.level < 4){ // should check if num_levels is less than 4 too
        for(uint i=0; i < 4; i++){
            cleanSide(*node.childs[i], num_levels);
        }
    }
}


static void async_texture_load(struct surface_node* node, struct planet_surface* surface){
    int n_channels;
    std::ostringstream fname;
    //bool res;

    fname << "../data/earth_textures/"
          << node->level << "_" 
          << (short)node->side << "_" 
          << node->x << "_"
          << node->y << ".png";

    node->data = stbi_load(fname.str().c_str(), &node->tex_x, &node->tex_y, &n_channels, 0);
    //res = read_png_f(fname.str().c_str(), node->tex_x, node->tex_y, node->data);
    /*if(!res){
        std::cout << "Failed to load texture " << fname.str() << std::endl;
    }*/


    fname.str("");
    fname.clear();

    fname << "../data/earth_textures/elevation/e_"
          << node->level << "_" 
          << (short)node->side << "_" 
          << node->x << "_"
          << node->y << ".png";

    node->data_elevation = stbi_load(fname.str().c_str(), &node->e_tex_x, &node->e_tex_y, &n_channels, 0);
    //res = read_png_f(fname.str().c_str(), node->e_tex_x, node->e_tex_y, node->data_elevation);
    /*if(!res){
        std::cout << "Failed to load texture " << fname.str() << std::endl;
    }*/

    surface->data_ready_mtx.lock();
    surface->data_ready_nodes.push_back(node);
    surface->data_ready_mtx.unlock();

    node->data_ready = true;
    node->loading = false;
}


void PlanetTree::bindLoadedTextures(){
    m_surface.data_ready_mtx.lock();
    for(uint i=0; i < m_surface.data_ready_nodes.size(); i++){
        bindLoadedTexture(*m_surface.data_ready_nodes.at(i));
    }
    m_surface.data_ready_nodes.clear();
    m_surface.data_ready_mtx.unlock();
}


void PlanetTree::textureFree(){
    std::vector<struct surface_node*>::iterator it = m_surface.bound_nodes.begin();

    while(it != m_surface.bound_nodes.end()){
        struct surface_node* node = *it;
        if(node->ticks_since_last_use >= 100){
            node->texture_loaded = false;
            glDeleteTextures(1, &node->tex_id);
            glDeleteTextures(1, &node->e_tex_id);
            it = m_surface.bound_nodes.erase(it);
            //std::cout << "Freed texture at level 3, side " << (short)node->side << ", x:" << node->x << ", y:" << node->y << " (currently loaded: " << bound_nodes.size() << ")" << std::endl;
        }
        else{
            node->ticks_since_last_use++;
            it++;
        }
    }

    check_gl_errors(true, "PlanetTree::textureFree");
}


void PlanetTree::loadBases(){
    PlanetTree::m_base32.reset(new Model("../data/base32.dae", nullptr, SHADER_PLANET, math::vec3(1.0, 1.0, 1.0)));
    PlanetTree::m_base64.reset(new Model("../data/base64.dae", nullptr, SHADER_PLANET, math::vec3(1.0, 1.0, 1.0)));
    //base128.reset(new Model("../data/base128.dae", nullptr, SHADER_PLANET, frustum, render_context, math::vec3(1.0, 1.0, 1.0)));
    PlanetTree::m_base32->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
    PlanetTree::m_base64->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
    //base128->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
}


void PlanetTree::renderSide(struct surface_node& node, const math::mat4& planet_transform_world,
                            int max_level, const dmath::vec3& cam_origin, double sea_level){
    // PRECOMPUTE THIS VVVVV
    dmath::vec3 path_translation_normd = dmath::vec3(dmath::quat_to_mat4(node.base_rotation) * dmath::vec4(dmath::normalise(node.patch_translation), 1.0)) * sea_level;
    double distance = dmath::distance(path_translation_normd, cam_origin);
    bool texture_is_loaded = true;

    if(node.scale * sea_level * 1.5 > distance && node.level < max_level){ // && false){
        for(uint i = 0; i < 4; i++){
            renderSide(*node.childs[i].get(), planet_transform_world, max_level, cam_origin, sea_level);
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
#ifdef ASYNC_PLANET_TEXTURE_LOAD
                std::thread thread(async_texture_load, node.uppermost_textured_parent, &m_surface);
                thread.detach();
#else
                async_texture_load(node.uppermost_textured_parent, &m_surface);
                bindLoadedTexture(*node.uppermost_textured_parent);
#endif
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
#ifdef ASYNC_PLANET_TEXTURE_LOAD
                std::thread thread(async_texture_load, &node, &m_surface);
                thread.detach();
#else
                async_texture_load(&node, &m_surface);
                bindLoadedTexture(node);
#endif
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

    check_gl_errors(true, "PlanetTree::renderSide");

    if(texture_is_loaded){
        glUniform2fv(m_tex_shift_location, 1, node.tex_shift.v);
        glUniform1f(m_texture_scale_location, node.texture_scale);
    }
    else{
        glUniform2fv(m_tex_shift_location, 1, node.tex_shift_lod.v);
        glUniform1f(m_texture_scale_location, node.texture_scale_lod);
    }
    if(node.level >= 1 && node.level < 3){
        PlanetTree::m_base32->render_terrain(planet_transform_world);
    }
    else if(node.level >= 3 && node.level < 7){ // 128x128 disabled
        PlanetTree::m_base64->render_terrain(planet_transform_world);
    }
    else{
        PlanetTree::m_base128->render_terrain(planet_transform_world);
    }
}


void PlanetTree::render(const dmath::vec3& cam_translation, const dmath::mat4 transform){
    if(!m_surface.is_built)
        return;

#ifdef ASYNC_PLANET_TEXTURE_LOAD
    bindLoadedTextures();
#endif
    math::mat4 planet_transform_world;
    dmath::mat4 dplanet_transform_world = transform;
    dplanet_transform_world.m[12] -= cam_translation.v[0];
    dplanet_transform_world.m[13] -= cam_translation.v[1];
    dplanet_transform_world.m[14] -= cam_translation.v[2];
    std::copy(dplanet_transform_world.m, dplanet_transform_world.m + 16, planet_transform_world.m);     

    m_render_context->useProgram(SHADER_PLANET);
    glUniform1f(m_planet_radius_location, m_surface.planet_sea_level);
    glUniform1i(m_planet_texture, TEXTURE_LOCATION);
    glUniform1i(m_elevation_texture, ELEVATION_LOCATION);

    dmath::vec3 cam_trans_local(0.0, 0.0, 0.0);
    cam_trans_local.v[0] -= dplanet_transform_world.m[12];
    cam_trans_local.v[1] -= dplanet_transform_world.m[13];
    cam_trans_local.v[2] -= dplanet_transform_world.m[14];

    check_gl_errors(true, "PlanetTree::render");

    // confusing mess of additions and subtractions of cameras and origins, seems to work but not
    // sure why or how. also there might be floating point arithmetic problems with the rendering
    // pipeline... (maybe not? we're rendering with planet_transform_world, which is actually
    // relative to the centered camera... change the name of the var?)

    for(uint i=0; i < 6; i++){
        //dmath::vec3 t(6300000.0, 0.0, 0.0);
        renderSide(m_surface.surface_tree[i], planet_transform_world, m_surface.max_levels, cam_trans_local, m_surface.planet_sea_level);
    }

    textureFree();
}



