#ifndef ASSETS_UTILS_HPP
#define ASSETS_UTILS_HPP

#include <memory>

#include "../AssetManager.hpp"


class btTriangleIndexVertexArray;
class btGImpactMeshShape;

struct iv_array;

/* I will come up with something better in the future, probably a friend class */

void load_parts(AssetManager& asset_manager);

int load_bullet_trimesh(std::unique_ptr<iv_array>& array, std::unique_ptr<btGImpactMeshShape>& shape, const std::string& file);

#endif