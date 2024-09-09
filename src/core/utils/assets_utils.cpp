#include <functional>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define BT_USE_DOUBLE_PRECISION
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include "assets_utils.hpp"
#include "../RenderContext.hpp"
#include "../Physics.hpp"
#include "../log.hpp"
#include "../../assets/parts/parts.hpp"
#include "../../assets/Model.hpp"

void load_parts_old(AssetManager& asset_manager){
    btQuaternion quat;
    quat.setEuler(0, 0, 0);
    typedef BasePartMap::iterator map_iterator;
    std::pair<map_iterator, bool> res;
    std::hash<std::string> str_hash;

    // P80 engine
    std::unique_ptr<btCollisionShape> cylinder_p80(new btCylinderShape(btVector3(1.5, 5.6051, 1)));

    std::unique_ptr<Model> p80_model(new Model("../data/vega/p80.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.75, 0.75, 0.75)));
    std::unique_ptr<VegaSolidEngine> p80(new VegaSolidEngine(p80_model.get(), asset_manager.m_physics, cylinder_p80.get(), 7408.0, 666, static_cast<AssetManagerInterface*>(&asset_manager)));
    p80->setColor(math::vec3(0.75, 0.75, 0.75));
    p80->setParentAttachmentPoint(math::vec3(0.0, 5.6051, 0.0), math::vec3(0.0, 0.0, 0.0));
    p80->addAttachmentPoint(math::vec3(0.0, -5.6051, 0.0), math::vec3(0.0, 0.0, 0.0));
    p80->setName(std::string("p80_") + std::to_string(666));
    p80->setFancyName("P80");
    p80->setCollisionGroup(CG_DEFAULT | CG_PART);
    p80->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
    p80->addResource({asset_manager.m_resources.at(str_hash("htpb")).get(), 88385.0f, 88385.0f});
    p80->setProperties(PART_HAS_ENGINE | PART_SEPARATES);


    res = asset_manager.m_master_parts.insert({666, std::move(p80)});

    if(!res.second){
        log("Failed to insert part with id ", 666, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert part with id " << 666 << " (collided with " << res.first->first << ")" << std::endl;
    }

    asset_manager.m_collision_shapes.push_back(std::move(cylinder_p80));
    asset_manager.m_models.push_back(std::move(p80_model));

    // Z23 engine
    std::unique_ptr<btCollisionShape> cylinder_z23(new btCylinderShape(btVector3(0.95, 3.4138, 1)));

    std::unique_ptr<Model> z23_model(new Model("../data/vega/z23.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.75, 0.75, 0.75)));
    std::unique_ptr<Model> z23_f_model(new Model("../data/vega/z23_fairing.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.75, 0.75, 0.75)));
    std::unique_ptr<VegaSolidEngine> z23(new VegaSolidEngine(z23_model.get(), asset_manager.m_physics, cylinder_z23.get(), 1860.0, 777, static_cast<AssetManagerInterface*>(&asset_manager)));
    z23->setColor(math::vec3(0.75, 0.75, 0.75));
    z23->setParentAttachmentPoint(math::vec3(0.0, 3.4138, 0.0), math::vec3(0.0, 0.0, 0.0));
    z23->addAttachmentPoint(math::vec3(0.0, -4.4054, 0.0), math::vec3(0.0, 0.0, 0.0));
    z23->setName(std::string("z23_") + std::to_string(777));
    z23->setFancyName("Z23");
    z23->setCollisionGroup(CG_DEFAULT | CG_PART);
    z23->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
    z23->addResource({asset_manager.m_resources.at(str_hash("htpb")).get(), 23900.0f, 23900.0f});
    z23->setProperties(PART_HAS_ENGINE | PART_SEPARATES);

    res = asset_manager.m_master_parts.insert({777, std::move(z23)});

    if(!res.second){
        log("Failed to insert part with id ", 777, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert part with id " << 777 << " (collided with " << res.first->first << ")" << std::endl;
    }
    asset_manager.m_collision_shapes.push_back(std::move(cylinder_z23));
    asset_manager.m_models.push_back(std::move(z23_f_model));
    asset_manager.m_models.push_back(std::move(z23_model));

    // Z9 engine
    std::unique_ptr<btCollisionShape> cylinder_z9(new btCylinderShape(btVector3(0.95, 1.2695, 1)));

    std::unique_ptr<Model> z9_model(new Model("../data/vega/z9.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.75, 0.75, 0.75)));
    std::unique_ptr<Model> z9_f_model(new Model("../data/vega/z9_fairing.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.75, 0.75, 0.75)));
    std::unique_ptr<VegaSolidEngine> z9(new VegaSolidEngine(z9_model.get(), asset_manager.m_physics, cylinder_z9.get(), 835.0, 888, static_cast<AssetManagerInterface*>(&asset_manager)));
    z9->setColor(math::vec3(0.75, 0.75, 0.75));
    z9->setParentAttachmentPoint(math::vec3(0.0, 1.2695, 0.0), math::vec3(0.0, 0.0, 0.0));
    z9->addAttachmentPoint(math::vec3(0.0, -1.8695, 0.0), math::vec3(0.0, 0.0, 0.0));
    z9->setName(std::string("z9_") + std::to_string(888));
    z9->setFancyName("Z9");
    z9->setCollisionGroup(CG_DEFAULT | CG_PART);
    z9->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
    z9->addResource({asset_manager.m_resources.at(str_hash("htpb")).get(), 10115.0f, 10115.0f});
    z9->setProperties(PART_HAS_ENGINE | PART_SEPARATES);


    res = asset_manager.m_master_parts.insert({888, std::move(z9)});

    if(!res.second){
        log("Failed to insert part with id ", 888, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert part with id " << 888 << " (collided with " << res.first->first << ")" << std::endl;
    }
    asset_manager.m_collision_shapes.push_back(std::move(cylinder_z9));
    asset_manager.m_models.push_back(std::move(z9_f_model));
    asset_manager.m_models.push_back(std::move(z9_model));

    // 3 m separator
    std::unique_ptr<btCollisionShape> separator3m_shape(new btCylinderShape(btVector3(1.5, 0.075, 1.5)));
    std::unique_ptr<Model> separator3m_model(new Model("../data/vega/3m_separator.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.75, 0.75, 0.75)));

    std::unique_ptr<Separator> separator3m(new Separator(separator3m_model.get(), asset_manager.m_physics, separator3m_shape.get(), 1000.0, 999, static_cast<AssetManagerInterface*>(&asset_manager)));
    separator3m->setColor(math::vec3(0.75, 0.75, 0.75));
    separator3m->setParentAttachmentPoint(math::vec3(0.0, 0.075, 0.0), math::vec3(0.0, 0.0, 0.0));
    separator3m->addAttachmentPoint(math::vec3(0.0, -0.075, 0.0), math::vec3(0.0, 0.0, 0.0));
    separator3m->setName(std::string("3m_separator_") + std::to_string(999));
    separator3m->setFancyName("3m separator");
    separator3m->setCollisionGroup(CG_DEFAULT | CG_PART);
    separator3m->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
    separator3m->setProperties(PART_SEPARATES);
    separator3m->setMaxForce(100000);

    res = asset_manager.m_master_parts.insert({999, std::move(separator3m)});

    if(!res.second){
        log("Failed to insert part with id ", 999, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert part with id " << 999 << " (collided with " << res.first->first << ")" << std::endl;
    }

    asset_manager.m_collision_shapes.push_back(std::move(separator3m_shape));
    asset_manager.m_models.push_back(std::move(separator3m_model));
}


int load_bullet_trimesh(std::unique_ptr<iv_array>& array, std::unique_ptr<btGImpactMeshShape>& shape, const std::string& file){
    int num_faces;
    int num_vertices;

    const aiMesh* mesh;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices);

    if(!scene){
        log("asset_utils::load_bullet_trimesh: could not open file ", file, " (", importer.GetErrorString(), ")");
        std::cerr << "asset_utils::load_bullet_trimesh: could not open file " << file << " (" << importer.GetErrorString() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    mesh = scene->mMeshes[0];
    num_faces = mesh->mNumFaces;
    num_vertices = mesh->mNumVertices;

    if(mesh->HasPositions()){
        array->points.reset(new btScalar[num_vertices * 3]);
        for(int i = 0; i < num_vertices; i++){
            const aiVector3D *vp = &(mesh->mVertices[i]);
            array->points[i * 3] = (btScalar)vp->x;
            array->points[i * 3 + 1] = (btScalar)vp->y;
            array->points[i * 3 + 2] = (btScalar)vp->z;
        }
    }
    if(mesh->HasFaces()){
        array->indices.reset(new int[num_faces * 3]);
        for(int i = 0; i < num_faces; i++){
            array->indices[i * 3] = mesh->mFaces[i].mIndices[0];
            array->indices[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
            array->indices[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
        }
    }

    array->bt_ivarray.reset(new btTriangleIndexVertexArray(num_faces,
                                                           array->indices.get(),
                                                           3*sizeof(int),
                                                           num_vertices,
                                                           array->points.get(),
                                                           3*sizeof(btScalar)));

    shape.reset(new btGImpactMeshShape(array->bt_ivarray.get()));
    //shape->setLocalScaling(btVector3(1.f,1.f,1.f));
    shape->updateBound();

    return EXIT_SUCCESS;
}

