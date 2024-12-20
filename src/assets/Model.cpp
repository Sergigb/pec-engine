#include <stb/stb_image.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Model.hpp"
#include "../core/RenderContext.hpp"
#include "../core/maths_funcs.hpp"
#include "../core/log.hpp"
#include "../core/utils/gl_utils.hpp"


const Frustum* Model::m_frustum;
const RenderContext* Model::m_render_context;

Model::Model(){
    m_has_texture = false;
}


Model::Model(const char* path_to_mesh, const char* path_to_texture, int shader, const math::vec3& mesh_color){
    m_shader = shader;

    m_model_mat_location = m_render_context->getUniformLocation(m_shader, "model");
    m_color_location = m_render_context->getUniformLocation(m_shader, "object_color");

    m_mesh_color = math::vec4(mesh_color, 1.0);

    loadScene(std::string(path_to_mesh));

    if(path_to_texture != nullptr){
        unsigned char* data;

        m_has_texture = true;

        data = stbi_load(path_to_texture, &m_tex_x, &m_tex_y, &m_n_channels, 0);

        glGenTextures(1, &m_tex_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_tex_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_tex_x, m_tex_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else{
        m_has_texture = false;
    }
    check_gl_errors(true, "Model::Model");
}


Model::~Model(){
    glDeleteBuffers(1, &m_vbo_vert);
    glDeleteBuffers(1, &m_vbo_tex);
    glDeleteBuffers(1, &m_vbo_ind);
    glDeleteBuffers(1, &m_vbo_norm);
    glDeleteVertexArrays(1, &m_vao);
    if(m_has_texture)
        glDeleteTextures(1, &m_tex_id);
    check_gl_errors(true, "Model::~Model");
}


void Model::setMeshColor(const math::vec4& mesh_color){
    m_mesh_color = mesh_color;
}


int Model::loadScene(const std::string& pFile){
    std::unique_ptr<GLfloat[]> points;
    std::unique_ptr<GLfloat[]> normals;
    std::unique_ptr<GLfloat[]> texcoords;
    std::unique_ptr<GLuint[]> indices;

    int num_vertices;
    const aiMesh* mesh;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(pFile, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices); // check for more flags

    if(!scene){
        log("Model::loadScene: Could not open file ", pFile, " (", importer.GetErrorString(), ")");
        std::cerr << "Model::loadScene: Could not open file " << pFile << " (" << importer.GetErrorString() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    mesh = scene->mMeshes[0];
    m_num_faces = mesh->mNumFaces;
    num_vertices = mesh->mNumVertices;

    if(mesh->HasPositions()){
        points.reset(new GLfloat[num_vertices * 3]);
        float norm;
        m_cs_radius = 0;
        for(int i = 0; i < num_vertices; i++){
            const aiVector3D *vp = &(mesh->mVertices[i]);
            points.get()[i * 3] = (GLfloat)vp->x;
            points.get()[i * 3 + 1] = (GLfloat)vp->y;
            points.get()[i * 3 + 2] = (GLfloat)vp->z;
            norm = std::sqrt(vp->x * vp->x + vp->y * vp->y + vp->z * vp->z);
            if(norm > m_cs_radius)
                m_cs_radius = norm;
        }
        m_aabb = get_AABB(points.get(), num_vertices);
    }
    if(mesh->HasNormals()){
        normals.reset(new GLfloat[num_vertices * 3]);
        for(int i = 0; i < num_vertices; i++){
            const aiVector3D *vn = &(mesh->mNormals[i]);
            normals.get()[i * 3] = (GLfloat)vn->x;
            normals.get()[i * 3 + 1] = (GLfloat)vn->y;
            normals.get()[i * 3 + 2] = (GLfloat)vn->z;
        }
    }
    if(mesh->HasTextureCoords(0)){
        texcoords.reset(new GLfloat[num_vertices * 2]);
        for(int i = 0; i < num_vertices; i++){
            const aiVector3D* vt = &(mesh->mTextureCoords[0][i]);
            texcoords.get()[i * 2] = (GLfloat)vt->x;
            texcoords.get()[i * 2 + 1] = (GLfloat)vt->y;
        }
    }
    if(mesh->HasFaces()){
        indices.reset(new unsigned int[m_num_faces * 3]);
        for(int i = 0; i < m_num_faces; i++){
            indices.get()[i * 3] = mesh->mFaces[i].mIndices[0];
            indices.get()[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
            indices.get()[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
        }
    }
    
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    if(mesh->HasPositions()){
        glGenBuffers(1, &m_vbo_vert);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
        glBufferData(GL_ARRAY_BUFFER, 3 * num_vertices * sizeof(GLfloat), points.get(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
    }
    if(mesh->HasNormals()){
        glGenBuffers(1, &m_vbo_norm);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo_norm);
        glBufferData(GL_ARRAY_BUFFER, 3 * num_vertices * sizeof(GLfloat), normals.get(), GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
    }
    if(mesh->HasTextureCoords(0)){
        glGenBuffers(1, &m_vbo_tex);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
        glBufferData(GL_ARRAY_BUFFER, 2 * num_vertices * sizeof(GLfloat), texcoords.get(), GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);
    }
    if(mesh->HasFaces()){
        glGenBuffers(1, &m_vbo_ind);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_ind);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * m_num_faces * sizeof(GLuint), indices.get(), GL_STATIC_DRAW);
    }

    if(mesh->HasTangentsAndBitangents()){
        // NB: could store/print tangents here
    }

    std::string msg = std::string("Model::loadScene") + pFile;

    check_gl_errors(true, msg.c_str());

    return EXIT_SUCCESS;
}


int Model::render(const math::mat4& transform) const{
    if(Model::m_frustum->checkBox(m_aabb.vert, transform)){
    ///if(m_frustum->checkSphere(math::vec3(transform.m[12], transform.m[13], transform.m[14]), m_cs_radius)){
        m_render_context->useProgram(m_shader);
        m_render_context->bindVao(m_vao);

        if(m_has_texture){
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_tex_id);
        }

        glUniform4fv(m_color_location, 1, m_mesh_color.v);
        glUniformMatrix4fv(m_model_mat_location, 1, GL_FALSE, transform.m);
        glDrawElements(GL_TRIANGLES, m_num_faces * 3, GL_UNSIGNED_INT, NULL);

        check_gl_errors(true, "Model::render(const math::mat4&)");

        return 1;
    }
    else{
        return 0;
    }
    return 0;
}


void Model::render_terrain(const math::mat4& transform) const{
    m_render_context->useProgram(m_shader);
    m_render_context->bindVao(m_vao);

    if(m_has_texture){
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_tex_id);
    }

    glUniform4fv(m_color_location, 1, m_mesh_color.v);
    glUniformMatrix4fv(m_model_mat_location, 1, GL_FALSE, transform.m);
    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glDrawElements(GL_TRIANGLES, m_num_faces * 3, GL_UNSIGNED_INT, NULL);

    check_gl_errors(true, "Model::render_terrain");
}



void Model::setStaticMembers(const Frustum* frustum,
                             const RenderContext* render_context){
    Model::m_frustum = frustum;
    Model::m_render_context = render_context;
}