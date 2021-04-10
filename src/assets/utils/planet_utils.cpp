#include <sstream>

#include <stb/stb_image.h>

#include "planet_utils.hpp"


void set_transform(struct surface_node& node, const struct surface_node& parent, int sign_side_1, int sign_side_2){
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


void bind_texture(struct surface_node& node){
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
}


void bind_elevation_texture(struct surface_node& node){
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
        node.childs[i]->ticks_since_last_use = 0;
        node.childs[i]->data_ready = false;
        node.childs[i]->has_elevation = true;
        node.childs[i]->tex_id_fl = node.tex_id_fl;
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

    set_transform(*node.childs[0].get(), node, 1, 1);
    set_transform(*node.childs[1].get(), node, -1, 1);
    set_transform(*node.childs[2].get(), node, 1, -1);
    set_transform(*node.childs[3].get(), node, -1, -1);

    if(node.level + 2 <= num_levels){ // node.level + 2 ----> build the childs of the childs you have just built... confusing
        for(uint i = 0; i < 4; i++){
            build_childs(*node.childs[i].get(), num_levels);
        }
    }
}


void bind_loaded_texture(struct surface_node& node, struct planet_surface& surface){
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

    surface.bound_nodes.push_back(&node);
    node.ticks_since_last_use = 0;
    node.texture_loaded = true;
    node.data_ready = false;
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
        surface.surface_tree[i].tex_shift_lod = math::vec2(0.0, 0.0);
        surface.surface_tree[i].scale = 1.0;
        surface.surface_tree[i].level = 1;
        surface.surface_tree[i].has_texture = true;
        surface.surface_tree[i].x = 0;
        surface.surface_tree[i].y = 0;
        surface.surface_tree[i].loading = false;
        surface.surface_tree[i].ticks_since_last_use = 0;
        surface.surface_tree[i].data_ready = false;
        surface.surface_tree[i].has_elevation = true;
        surface.surface_tree[i].texture_scale = 1.0f;
        surface.surface_tree[i].texture_scale_lod = 1.0f;
        surface.surface_tree[i].uppermost_textured_parent = &surface.surface_tree[i];
        bind_texture(surface.surface_tree[i]);
        surface.surface_tree[i].tex_id_fl = surface.surface_tree[i].tex_id;
        bind_elevation_texture(surface.surface_tree[i]);
        surface.surface_tree[i].e_tex_id_fl = surface.surface_tree[i].e_tex_id;
        build_childs(surface.surface_tree[i], num_levels);
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


void async_texture_load(struct surface_node* node, struct planet_surface* surface){
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

    surface->data_ready_mtx.lock();
    surface->data_ready_nodes.push_back(node);
    surface->data_ready_mtx.unlock();

    node->data_ready = true;
    node->loading = false;
}


void bind_loaded_textures(struct planet_surface& surface){
    surface.data_ready_mtx.lock();
    //std::cout << data_ready_nodes.size() << std::endl;
    for(uint i=0; i < surface.data_ready_nodes.size(); i++){
        bind_loaded_texture(*surface.data_ready_nodes.at(i), surface);
    }
    surface.data_ready_nodes.clear();
    surface.data_ready_mtx.unlock();
}


void texture_free(struct planet_surface& surface){
    std::vector<struct surface_node*>::iterator it = surface.bound_nodes.begin();

    while(it != surface.bound_nodes.end()){
        struct surface_node* node = *it;
        if(node->ticks_since_last_use >= 100){
            node->texture_loaded = false;
            glDeleteTextures(1, &node->tex_id);
            glDeleteTextures(1, &node->e_tex_id);
            it = surface.bound_nodes.erase(it);
            //std::cout << "Freed texture at level 3, side " << (short)node->side << ", x:" << node->x << ", y:" << node->y << " (currently loaded: " << bound_nodes.size() << ")" << std::endl;
        }
        else{
            node->ticks_since_last_use++;
            it++;
        }
    }
}


btVector3 reference_ellipse_to_xyz(btScalar latitude, btScalar longitude, btScalar radius){
    btVector3 loc;

    // Longitude seems to be off when it's not 0, the latitude seems to be right
    loc.setX(btCos(latitude) * btCos(longitude));
    loc.setY(btCos(latitude) * btSin(longitude));
    loc.setZ(-btSin(latitude));
    loc *= radius;

    return loc;
}

