#ifndef ASSETS_UTILS_HPP
#define ASSETS_UTILS_HPP

#include <memory>

#include "../AssetManager.hpp"


class btTriangleIndexVertexArray;
class btGImpactMeshShape;

struct iv_array;

/*
 * Legacy function to "load" the parts available for the game. In the near future I should load the
 * parts like the way I do for the planets and the resources, from xml files.
 *
 * @asset_manager: reference to the asset manager of the game.
 */
void load_parts(AssetManager& asset_manager);


/*
 * Loads a triangle mesh from a valid 3D file. It stores the mesh in a struct of type iv_array, 
 * defined in the header Physics.hpp, check that file for more detailed information of this struct.
 * The mesh is used by a GImpact collition shape object, it allows us to have kinematic objects 
 * with a collision shape defined by a mesh shape (which allows concavities).
 *
 * @array: reference to the unique pointer of the iv_array where we want to store the mesh.
 * @shape: reference to the unique pointer of the GImpact mesh shape that is going to use the 
 * triangle mesh.
 * @file: constant reference to a string with the path to the mesh file.
 */
int load_bullet_trimesh(std::unique_ptr<iv_array>& array, std::unique_ptr<btGImpactMeshShape>& shape, const std::string& file);

#endif
