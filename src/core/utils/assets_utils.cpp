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

