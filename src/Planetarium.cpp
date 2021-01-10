#include <string>
#include <iostream>
#include <thread>
#include <math.h>
#include <mutex>
#include <sstream>

#include <stb/stb_image.h>

#include "Planetarium.hpp"
#include "Model.hpp"
#include "maths_funcs.hpp"
#include "FontAtlas.hpp"
#include "RenderContext.hpp"
#include "Camera.hpp"
#include "WindowHandler.hpp"
#include "Input.hpp"
#include "Player.hpp"
#include "planet_utils.hpp"
#include "Planet.hpp"

//#include "read_png.hpp"


std::vector<struct surface_node*> bound_nodes; // this should go into planet_surface but really I don't care right now
std::vector<struct surface_node*> data_ready_nodes;
std::mutex data_ready_mtx;


Planetarium::Planetarium() : BaseApp(){
    init();
}


Planetarium::Planetarium(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


void Planetarium::init(){
    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);
}


Planetarium::~Planetarium(){
}


void async_texture_load(struct surface_node* node){
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

    //std::cout << "Loaded texture: " << fname.str() << std::endl;

    data_ready_mtx.lock();
    data_ready_nodes.push_back(node);
    data_ready_mtx.unlock();

    node->data_ready = true;
    node->loading = false;
}


void bind_loaded_texture(struct surface_node& node){
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

    bound_nodes.push_back(&node);
    node.ticks_since_last_use = 0;
    node.texture_loaded = true;
    node.data_ready = false;
}


void bind_loaded_textures(){
    data_ready_mtx.lock();
    //std::cout << data_ready_nodes.size() << std::endl;
    for(uint i=0; i < data_ready_nodes.size(); i++){
        bind_loaded_texture(*data_ready_nodes.at(i));
    }
    data_ready_nodes.clear();
    data_ready_mtx.unlock();
}


GLuint relative_planet_location, texture_scale_location, tex_shift_location;

std::unique_ptr<Model> base32;
std::unique_ptr<Model> base64;
std::unique_ptr<Model> base128;


// ugly but temporal
void Planetarium::render_side(struct surface_node& node, math::mat4& planet_transform_world, int max_level, dmath::vec3& cam_origin, double sea_level){
    // path_translation_normd should be precomputed
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
                //std::thread thread(async_texture_load, node.uppermost_textured_parent);
                //thread.detach();
                async_texture_load(node.uppermost_textured_parent);
                bind_loaded_texture(*node.uppermost_textured_parent);
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
                //std::thread thread(async_texture_load, &node);
                //thread.detach();
                async_texture_load(&node);
                bind_loaded_texture(node);
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

    glUniformMatrix4fv(relative_planet_location, 1, GL_FALSE, transform_planet_relative.m);

    if(texture_is_loaded){
        glUniform2fv(tex_shift_location, 1, node.tex_shift.v);
        glUniform1f(texture_scale_location, node.texture_scale);
    }
    else{
        glUniform2fv(tex_shift_location, 1, node.tex_shift_lod.v);
        glUniform1f(texture_scale_location, node.texture_scale_lod);
    }

    if(node.level >= 1 && node.level < 3){
        base32->render_terrain(planet_transform_world);
    }
    else if(node.level >= 3 && node.level < 7){ // 128x128 disabled
        base64->render_terrain(planet_transform_world);
    }
    else{
        base128->render_terrain(planet_transform_world);
    }
    
}


void texture_free(){
    std::vector<struct surface_node*>::iterator it = bound_nodes.begin();

    while(it != bound_nodes.end()){
        struct surface_node* node = *it;
        if(node->ticks_since_last_use >= 100){
            node->texture_loaded = false;
            glDeleteTextures(1, &node->tex_id);
            glDeleteTextures(1, &node->e_tex_id);
            it = bound_nodes.erase(it);
            //std::cout << "Freed texture at level 3, side " << (short)node->side << ", x:" << node->x << ", y:" << node->y << " (currently loaded: " << bound_nodes.size() << ")" << std::endl;
        }
        else{
            node->ticks_since_last_use++;
            it++;
        }
    }
}


void clean_side(struct surface_node& node, int num_levels){ // num_levels should be a macro in the future
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
            clean_side(*node.childs[i], num_levels);
        }
    }
}


void Planetarium::run(){
    bool polygon_mode_lines = false;

    base32.reset(new Model("../data/base32.dae", nullptr, SHADER_PLANET, m_frustum.get(), m_render_context.get(), math::vec3(1.0, 1.0, 1.0)));
    base64.reset(new Model("../data/base64.dae", nullptr, SHADER_PLANET, m_frustum.get(), m_render_context.get(), math::vec3(1.0, 1.0, 1.0)));
    base128.reset(new Model("../data/base128.dae", nullptr, SHADER_PLANET, m_frustum.get(), m_render_context.get(), math::vec3(1.0, 1.0, 1.0)));
    base32->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
    base64->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
    base128->setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
    dmath::mat4 planet_transform = dmath::identity_mat4();

    struct planet_surface surface;
    surface.max_levels = 6;
    surface.planet_sea_level = 6371000.f;
    plutils::build_surface(surface);

    m_camera->setCameraPosition(dmath::vec3(9300000.0, 0.0, 0.0));
    m_camera->setSpeed(630000.0f);
    m_camera->createProjMat(1.0, 63000000, 67.0, 1.0);

    //glDisable(GL_CULL_FACE);

    m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));

    relative_planet_location = m_render_context->getUniformLocation(SHADER_PLANET, "relative_planet");
    texture_scale_location = m_render_context->getUniformLocation(SHADER_PLANET, "texture_scale");
    tex_shift_location = m_render_context->getUniformLocation(SHADER_PLANET, "tex_shift");
    GLuint planet_radius_location = m_render_context->getUniformLocation(SHADER_PLANET, "planet_radius");

    // texture location setup
    GLuint planet_texture = m_render_context->getUniformLocation(SHADER_PLANET, "tex");
    GLuint elevation_texture = m_render_context->getUniformLocation(SHADER_PLANET, "elevation");

    m_render_context->useProgram(SHADER_PLANET);
    glUniform1i(planet_texture, TEXTURE_LOCATION);
    glUniform1i(elevation_texture, ELEVATION_LOCATION);

    m_render_context->toggleDebugOverlay();

    while (!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
        m_player->update();

        m_render_context->contextUpdatePlanetarium();

        dmath::vec3 cam_translation = m_camera->getCamPosition();

        ////////////////////

        //bind_loaded_textures();

        math::mat4 planet_transform_world;
        dmath::mat4 dplanet_transform_world = planet_transform;
        dplanet_transform_world.m[12] -= cam_translation.v[0];
        dplanet_transform_world.m[13] -= cam_translation.v[1];
        dplanet_transform_world.m[14] -= cam_translation.v[2];
        std::copy(dplanet_transform_world.m, dplanet_transform_world.m + 16, planet_transform_world.m);     

        m_render_context->useProgram(SHADER_PLANET);
        glUniform1f(planet_radius_location, surface.planet_sea_level);
        //glfwSwapInterval(0);

        if(m_input->pressed_keys[GLFW_KEY_R] & INPUT_KEY_RELEASE){
            m_render_context->reloadShaders();
            std::cout << "Shaders reloaded" << std::endl;
        }

        if(m_input->pressed_keys[GLFW_KEY_L] & INPUT_KEY_RELEASE){
            if(polygon_mode_lines){
                polygon_mode_lines = false;
            }
            else{
                polygon_mode_lines = true;
            }
        }

        if(polygon_mode_lines){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else{
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        for(uint i=0; i < 6; i++){
            //dmath::vec3 t(6300000.0, 0.0, 0.0);
            render_side(surface.surface_tree[i], planet_transform_world, surface.max_levels, cam_translation, surface.planet_sea_level);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        texture_free();

        //m_render_context->setLightPosition(math::vec3(cam_translation.v[0], cam_translation.v[1], cam_translation.v[2]));

        glfwSwapBuffers(m_window_handler->getWindow());
    }

    for(uint i=0; i < 6; i++){
        clean_side(surface.surface_tree[i], surface.max_levels);
    }

    m_window_handler->terminate();
}

