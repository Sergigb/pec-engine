#include <string>
#include <iostream>
#include <thread>

#include <stb/stb_image.h>

#include "Planetarium.hpp"
#include "Model.hpp"
#include "maths_funcs.hpp"


std::vector<struct surface_node*> l3_loaded_nodes; // this should go into planet_surface but really I don't care right now


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


#include <math.h>       /* pow */
void set_transform(struct surface_node& node, const struct surface_node& parent, int sign_side_1, int sign_side_2){
    node.patch_translation = parent.patch_translation;
    node.patch_translation.v[1] += (node.scale / 2) * sign_side_1;
    node.patch_translation.v[2] += (node.scale / 2) * sign_side_2;
    if(node.level <= 3){
        node.tex_shift.v[0] = 0.0f;
        node.tex_shift.v[1] = 0.0f;
    }
    else{
        short scale = pow(2.0, short(node.level - node.uppermost_textured_parent->level) + 1);

        node.tex_shift.v[0] = parent.tex_shift.v[0] + (1.0 / scale) * sign_side_1;
        node.tex_shift.v[1] = parent.tex_shift.v[1] + (1.0 / scale) * sign_side_2;
        print(node.tex_shift);
        std::cout << (1.0 / scale) * sign_side_1 << ", " << (1.0 / scale) * sign_side_2 << std::endl;;
    }

}


/*

  texture name: level_side_x_y

  frist level: 1_0_0_0.png, 1_1_0_0.png, 1_2_0_0.png, 1_3_0_0.png
  second level: 2_0_0_0.png, 2_0_0_1.png, 2_0_1_0.png, 2_0_1_1.png, 2_1_0_0.png, 2_1_0_1.png, 2_1_1_0.png...

dont pay too much attention to this

  levels with texture:
    -level 1 - 1k - always loaded
    -level 2 - 1k - always loaded
    -level 3 - 1k - will be dynamically loaded
    -level 4 - no texture
    -level 5 - no texture
    -level 6 - 1k - will be dynamically loaded


*/


void bind_texture(struct surface_node& node){
    int tex_x, tex_y, n_channels;
    std::ostringstream fname;

    if(node.level <= 2){
        glGenTextures(1, &node.tex_id);
        glBindTexture(GL_TEXTURE_2D, node.tex_id);
        fname << "../data/earth_textures/"
              << node.level << "_" 
              << (short)node.side << "_" 
              << node.x << "_"
              << node.y << ".png";
        node.texture_loaded = true;
    }
    else{
        glGenTextures(1, &node.tex_id_lod);
        glBindTexture(GL_TEXTURE_2D, node.tex_id_lod);
        fname << "../data/earth_textures/thumb_"
              << node.level << "_" 
              << (short)node.side << "_" 
              << node.x << "_"
              << node.y << ".png";
        node.texture_loaded = false;
    }

    node.data = stbi_load(fname.str().c_str(), &tex_x, &tex_y, &n_channels, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_x, tex_y, 0, GL_RGB, GL_UNSIGNED_BYTE, node.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    stbi_image_free(node.data);
}


void bind_elevation_texture(struct surface_node& node){
    int tex_x, tex_y, n_channels;
    std::ostringstream fname;

    if(node.level <= 9999){  // load all levels
        glGenTextures(1, &node.e_tex_id);
        glBindTexture(GL_TEXTURE_2D, node.e_tex_id);
        fname << "../data/earth_textures/elevation/e_"
              << node.level << "_" 
              << (short)node.side << "_" 
              << node.x << "_"
              << node.y << ".png";
    }
    else{
        glGenTextures(1, &node.e_tex_id_lod);
        glBindTexture(GL_TEXTURE_2D, node.e_tex_id_lod);
        fname << "../data/earth_textures/elevation/thumb_e_"
              << node.level << "_" 
              << (short)node.side << "_" 
              << node.x << "_"
              << node.y << ".png";
    }

    unsigned char* data = stbi_load(fname.str().c_str(), &tex_x, &tex_y, &n_channels, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_x, tex_y, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    stbi_image_free(data);
}


void build_childs(struct surface_node& node, int num_levels){

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
        node.childs[i]->tiks_since_last_use = 0;
        node.childs[i]->data_ready = false;
        node.childs[i]->has_elevation = true;
        if(node.level + 1 < 4){
            node.childs[i]->texture_scale = 1.0;
            bind_texture(*node.childs[i]);
            bind_elevation_texture(*node.childs[i]);
            node.childs[i]->uppermost_textured_parent = node.childs[i].get();
        }
        else {
            node.childs[i]->texture_scale = node.texture_scale / 2.0;
            node.childs[i]->uppermost_textured_parent = node.uppermost_textured_parent;
        }

    }

    set_transform(*node.childs[0].get(), node, 1, 1);
    set_transform(*node.childs[1].get(), node, -1, 1);
    set_transform(*node.childs[2].get(), node, 1, -1);
    set_transform(*node.childs[3].get(), node, -1, -1);

    if(node.level + 2 <= num_levels){
        for(uint i = 0; i < 4; i++){
            build_childs(*node.childs[i].get(), num_levels);
        }
    }
}


void build_surface(struct planet_surface& surface){
    short num_levels = surface.max_levels;

    surface.surface_tree[0].side = SIDE_PX;
    surface.surface_tree[1].side = SIDE_NX;
    surface.surface_tree[2].side = SIDE_PY;
    surface.surface_tree[3].side = SIDE_NY;
    surface.surface_tree[4].side = SIDE_PZ;
    surface.surface_tree[5].side = SIDE_NZ;
    
    surface.surface_tree[0].base_rotation = dmath::quat_from_axis_rad(0.0, 1.0, 0.0, 0.0);
    surface.surface_tree[1].base_rotation = dmath::quat_from_axis_rad(M_PI, 0.0, 1.0, 0.0);
    surface.surface_tree[2].base_rotation = dmath::quat_from_axis_rad(M_PI/2, 0.0, 0.0, 1.0);
    surface.surface_tree[3].base_rotation = dmath::quat_from_axis_rad(-M_PI/2, 0.0, 0.0, 1.0);
    surface.surface_tree[4].base_rotation = dmath::quat_from_axis_rad(M_PI/2, 0.0, 1.0, 0.0);
    surface.surface_tree[5].base_rotation = dmath::quat_from_axis_rad(-M_PI/2, 0.0, 1.0, 0.0);

    for(uint i=0; i < 6; i++){
        surface.surface_tree[i].patch_translation = dmath::vec3(0.5, 0.0, 0.0);
        surface.surface_tree[i].tex_shift = math::vec2(0.0, 0.0);
        surface.surface_tree[i].scale = 1.0;
        surface.surface_tree[i].level = 1;
        surface.surface_tree[i].has_texture = true;
        surface.surface_tree[i].x = 0;
        surface.surface_tree[i].y = 0;
        surface.surface_tree[i].texture_loaded = false;
        surface.surface_tree[i].loading = false;
        surface.surface_tree[i].tiks_since_last_use = 0;
        surface.surface_tree[i].data_ready = false;
        surface.surface_tree[i].has_elevation = true;
        surface.surface_tree[i].texture_scale = 1.0;
        surface.surface_tree[i].uppermost_textured_parent = &surface.surface_tree[i];
        bind_texture(surface.surface_tree[i]);
        bind_elevation_texture(surface.surface_tree[i]);
        build_childs(surface.surface_tree[i], num_levels);
    }
}


void async_texture_load(struct surface_node* node){
    int n_channels;
    std::ostringstream fname;

    fname << "../data/earth_textures/"
          << node->level << "_" 
          << (short)node->side << "_" 
          << node->x << "_"
          << node->y << ".png";

    node->data = stbi_load(fname.str().c_str(), &node->tex_x, &node->tex_y, &n_channels, 0);

    //std::cout << "Loaded texture: " << fname.str() << std::endl;

    node->data_ready = true;
    node->loading = false;
}


void bind_loaded_texture(struct surface_node& node){
    glGenTextures(1, &node.tex_id);
    glActiveTexture(TEXTURE_LOCATION);
    glBindTexture(GL_TEXTURE_2D, node.tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, node.tex_x, node.tex_y, 0, GL_RGB, GL_UNSIGNED_BYTE, node.data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    //std::cout << "Texture bound: " << std::endl;

    stbi_image_free(node.data);

    l3_loaded_nodes.push_back(&node);
    node.tiks_since_last_use = 0;
    node.texture_loaded = true;
    node.data_ready = false;
}


GLuint relative_planet_location, texture_scale_location, tex_shift_location;


// ugly but temporal
void Planetarium::render_side(struct surface_node& node, Model& model, math::mat4& planet_transform_world, int max_level, dmath::vec3& cam_origin, double sea_level){
    // path_translation_normd should be precomputed
    dmath::vec3 path_translation_normd = dmath::vec3(dmath::quat_to_mat4(node.base_rotation) * dmath::vec4(dmath::normalise(node.patch_translation), 1.0)) * sea_level;
    double distance = dmath::distance(path_translation_normd, cam_origin);
    if(node.scale * sea_level * 1.5 > distance && node.level < max_level){
        for(uint i = 0; i < 4; i++){
            render_side(*node.childs[i].get(), model, planet_transform_world, max_level, cam_origin, sea_level);
        }
        return;
    }

    glActiveTexture(TEXTURE_LOCATION);

    if(node.level > 3){
        if(node.uppermost_textured_parent->texture_loaded){
            glBindTexture(GL_TEXTURE_2D, node.uppermost_textured_parent->tex_id);
            node.uppermost_textured_parent->tiks_since_last_use = 0;  
        }
        else{
            if(!node.uppermost_textured_parent->loading && !node.uppermost_textured_parent->data_ready){
                node.uppermost_textured_parent->loading = true;
                std::thread thread(async_texture_load, node.uppermost_textured_parent);
                thread.detach();
            }
            else if(node.uppermost_textured_parent->data_ready){
                bind_loaded_texture(*node.uppermost_textured_parent);
            }

            glBindTexture(GL_TEXTURE_2D, node.uppermost_textured_parent->tex_id_lod);
        }
        glActiveTexture(ELEVATION_LOCATION);
        glBindTexture(GL_TEXTURE_2D, node.uppermost_textured_parent->e_tex_id);
    }
    else{
        if(node.texture_loaded){
            glBindTexture(GL_TEXTURE_2D, node.tex_id);
            node.tiks_since_last_use = 0;
        }
        else{
            if(!node.loading && !node.data_ready){
                node.loading = true;
                std::thread thread(async_texture_load, &node);
                thread.detach();
            }
            else if(node.data_ready){
                bind_loaded_texture(node);
            }

            glBindTexture(GL_TEXTURE_2D, node.tex_id_lod);
        }
        glActiveTexture(ELEVATION_LOCATION);
        glBindTexture(GL_TEXTURE_2D, node.e_tex_id);
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

    glUniform1f(texture_scale_location, node.texture_scale);
    glUniformMatrix4fv(relative_planet_location, 1, GL_FALSE, transform_planet_relative.m);
    glUniform2fv(tex_shift_location, 1, node.tex_shift.v);

    model.setMeshColor(math::vec4(0.0, 0.0, 0.0, 1.0));
    model.render_terrain(planet_transform_world);
}


void texture_free(){
    std::vector<struct surface_node*>::iterator it = l3_loaded_nodes.begin();

    while(it != l3_loaded_nodes.end()){
        struct surface_node* node = *it;
        if(node->tiks_since_last_use >= 100){
            glDeleteTextures(1, &node->tex_id);
            node->texture_loaded = false;
            it = l3_loaded_nodes.erase(it);
            //std::cout << "Freed texture at level 3, side " << (short)node->side << ", x:" << node->x << ", y:" << node->y << " (currently loaded: " << l3_loaded_nodes.size() << ")" << std::endl;
        }
        else{
            node->tiks_since_last_use++;
            it++;
        }
    }
}


void Planetarium::run(){
    Model base("../data/base64.dae", nullptr, m_render_context->getShader(SHADER_PLANET), m_frustum.get(), m_render_context.get(), math::vec3(1.0, 1.0, 1.0));
    dmath::mat4 planet_transform = dmath::identity_mat4();

    struct planet_surface surface;
    surface.max_levels = 6;
    surface.planet_sea_level = 6371000.f;
    build_surface(surface);

    m_camera->setCameraPosition(dmath::vec3(9300000.0, 0.0, 0.0));
    m_camera->setSpeed(630000.0f);
    m_camera->createProjMat(1.0, 63000000, 67.0, 1.0);

    //glDisable(GL_CULL_FACE);

    m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));

    GLuint shader = m_render_context->getShader(SHADER_PLANET);
    relative_planet_location = glGetUniformLocation(shader, "relative_planet");
    texture_scale_location = glGetUniformLocation(shader, "texture_scale");
    tex_shift_location = glGetUniformLocation(shader, "tex_shift");
    GLuint planet_radius_location = glGetUniformLocation(shader, "planet_radius");

    // texture location setup
    GLuint planet_texture = glGetUniformLocation(shader, "tex");
    GLuint elevation_texture  = glGetUniformLocation(shader, "elevation");

    glUseProgram(shader);
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

        math::mat4 planet_transform_world;
        dmath::mat4 dplanet_transform_world = planet_transform;
        dplanet_transform_world.m[12] -= cam_translation.v[0];
        dplanet_transform_world.m[13] -= cam_translation.v[1];
        dplanet_transform_world.m[14] -= cam_translation.v[2];
        std::copy(dplanet_transform_world.m, dplanet_transform_world.m + 16, planet_transform_world.m);     

        m_render_context->useProgram(shader);
        glUniform1f(planet_radius_location, surface.planet_sea_level);
        //glfwSwapInterval(0);

        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        for(uint i=0; i < 6; i++){
            //dmath::vec3 t(6300000.0, 0.0, 0.0);

            render_side(surface.surface_tree[i], base, planet_transform_world, surface.max_levels, cam_translation, surface.planet_sea_level);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        texture_free();

        //m_render_context->setLightPosition(math::vec3(cam_translation.v[0], cam_translation.v[1], cam_translation.v[2]));

        glfwSwapBuffers(m_window_handler->getWindow());
    }

    m_window_handler->terminate();
}

