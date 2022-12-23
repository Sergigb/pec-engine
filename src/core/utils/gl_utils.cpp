#include <iostream>
#include <limits>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "gl_utils.hpp"
#include "utils.hpp"
#include "../log.hpp"
#include "../common.hpp"


void log_shader_info(GLuint shader_index){
    int actual_length = 0;
    char message[LOG_GL_MAX_LENGTH];
    glGetShaderInfoLog(shader_index, LOG_GL_MAX_LENGTH, &actual_length, message);
    //std::cout << "Shader info log for shader with index " << shader_index << ": " << message << std::endl;
    log("log_shader_info: Shader info log for shader with index ", shader_index, ": ", message);
}


void log_programme_info(GLuint program_index){
    int actual_length = 0;
    char message[LOG_GL_MAX_LENGTH];
    glGetProgramInfoLog(program_index, LOG_GL_MAX_LENGTH, &actual_length, message);
    //std::cout << "Program info log for program with index index " << program_index << ": " << message << std::endl;
    log("log_programme_info: Program info log for program with index index ",
        program_index, ": ", message);
}


int log_validate_programme(GLuint program_index){
    GLint params = -1;
    glValidateProgram(program_index);
    glGetProgramiv(program_index, GL_VALIDATE_STATUS, &params);
    if(GL_TRUE != params){
        log("log_validate_programme: Program ", program_index, 
            " GL_VALIDATE_STATUS = GL_FALSE, check the log");
        log_programme_info(program_index);
        return EXIT_FAILURE;
    }
    log("log_validate_programme: Program ", program_index, " GL_VALIDATE_STATUS = GL_TRUE");
    return EXIT_SUCCESS;
}


int create_shader(const char* filename, GLuint &shader, GLenum type){
    std::string shader_string;
    const GLchar* p;
    int params = -1;

    log("create_shader: Creating shader from file ", filename);

    file_to_str(filename, shader_string);
    shader = glCreateShader(type);
    p = (const GLchar*)shader_string.c_str();
    glShaderSource(shader, 1, &p, NULL);
    glCompileShader(shader);

    // check for compile errors
    glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
    if(params != GL_TRUE){
        log("create_shader: ERROR: GL shader index ", shader,
            " did not compile (file ", filename, ")");
        std::cerr << "create_shader: ERROR: GL shader index " << shader
                  << " did not compile (file " << filename << ")" << std::endl;
        log_shader_info(shader);
        return EXIT_FAILURE;
    }
    log("create_shader: Shader with index ", shader, " compiled (file ", filename, ")");
    return EXIT_SUCCESS;
}


int create_programme(GLuint vert, GLuint frag, GLuint &programme_index){
    GLint params = -1;

    programme_index = glCreateProgram();
    log("create_programme: Created programme ", programme_index,
        ", attaching shaders ", vert, " and ", frag);
    glAttachShader(programme_index, vert);
    glAttachShader(programme_index, frag);
    glLinkProgram(programme_index);
    
    glGetProgramiv(programme_index, GL_LINK_STATUS, &params);
    if(GL_TRUE != params){
        log("create_programme: ERROR: could not link shader programme GL index ", programme_index);
        std::cerr << "create_programme: ERROR: GL programme index " << programme_index
                  << " did not compile" << std::endl;
        log_programme_info(programme_index);
        return EXIT_FAILURE;
    }
    log_validate_programme(programme_index);
    // delete shaders here to free memory
    glDeleteShader(vert);
    glDeleteShader(frag);

    return EXIT_SUCCESS;
}


GLuint create_programme_from_files(const char* vert_file_name, const char* frag_file_name){
    GLuint vert, frag, programme;
    create_shader(vert_file_name, vert, GL_VERTEX_SHADER);
    create_shader(frag_file_name, frag, GL_FRAGMENT_SHADER);
    create_programme(vert, frag, programme);
    return programme;
}


int load_scene(const std::string pFile, GLuint& vao, int& point_count, float& cs_radius, struct bbox& aabb){
    GLfloat* points = nullptr;
    GLfloat* normals = nullptr;
    GLfloat* texcoords = nullptr;
    GLuint* indices  = nullptr;
    int num_vertices;
    const aiMesh* mesh;
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(pFile, aiProcess_Triangulate | aiProcess_JoinIdenticalVertices); // check for more flags

    if(!scene){
        log("load_scene: Could not open file ", pFile, " (", importer.GetErrorString(), ")");
        std::cerr << "load_scene: Could not open file " << pFile << " (" 
                  << importer.GetErrorString() << ")" << std::endl;
        return EXIT_FAILURE;
    }

    mesh = scene->mMeshes[0];
    point_count = mesh->mNumFaces; // the total number of vertices of the mesh comes from the number of triangles*3!
    num_vertices = mesh->mNumVertices; // not the absolute TOTAL number of vertices, as some of them are re-used!
    //std::cout << "total number of indices: " << point_count << ", total number of vertexes: " << num_vertices * 3  << std::endl;

    if(mesh->HasPositions()){
        points = new GLfloat[num_vertices * 3];
        float norm;
        cs_radius = 0;
        for(int i = 0; i < num_vertices; i++){
            const aiVector3D *vp = &(mesh->mVertices[i]);
            points[i * 3] = (GLfloat)vp->x;
            points[i * 3 + 1] = (GLfloat)vp->y;
            points[i * 3 + 2] = (GLfloat)vp->z;
            norm = std::sqrt(vp->x * vp->x + vp->y * vp->y + vp->z * vp->z);
            if(norm > cs_radius)
                cs_radius = norm;
        }
        aabb = get_AABB(points, num_vertices);
    }
    if(mesh->HasNormals()){
        normals = new GLfloat[num_vertices * 3];
        for(int i = 0; i < num_vertices; i++){
            const aiVector3D *vn = &(mesh->mNormals[i]);
            normals[i * 3] = (GLfloat)vn->x;
            normals[i * 3 + 1] = (GLfloat)vn->y;
            normals[i * 3 + 2] = (GLfloat)vn->z;
        }
    }
    if(mesh->HasTextureCoords(0)){
        texcoords = new GLfloat[num_vertices * 2];
        for(int i = 0; i < num_vertices; i++){
            const aiVector3D* vt = &(mesh->mTextureCoords[0][i]);
            texcoords[i * 2] = (GLfloat)vt->x;
            texcoords[i * 2 + 1] = (GLfloat)vt->y;
        }
    }
    if(mesh->HasFaces()){
        indices = new unsigned int[point_count * 3];
        for(int i = 0; i < point_count; i++){
            indices[i * 3] = mesh->mFaces[i].mIndices[0];
            indices[i * 3 + 1] = mesh->mFaces[i].mIndices[1];
            indices[i * 3 + 2] = mesh->mFaces[i].mIndices[2];
        }
    }
    
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    /* copy mesh data into VBOs */
    if(mesh->HasPositions()){
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * num_vertices * sizeof(GLfloat), points,
                                    GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(0);
        delete[] points;
    }
    if(mesh->HasNormals()){
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 3 * num_vertices * sizeof(GLfloat), normals,
                                    GL_STATIC_DRAW);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(1);
        delete[] normals;
    }
    if(mesh->HasTextureCoords(0)){
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 2 * num_vertices * sizeof(GLfloat), texcoords,
                                    GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(2);
        delete[] texcoords;
    }
    if(mesh->HasFaces()){
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * point_count * sizeof(GLuint), indices, GL_STATIC_DRAW);
        glVertexAttribPointer(3, 3, GL_UNSIGNED_INT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(3);
        delete[] indices;

    }

    if(mesh->HasTangentsAndBitangents()){
        // NB: could store/print tangents here
    }

    return EXIT_SUCCESS;
}


void update_fps_counter(GLFWwindow* window){
    static double previous_seconds = glfwGetTime();
    static int frame_count;
    double current_seconds = glfwGetTime();
    double elapsed_seconds = current_seconds - previous_seconds;
    if(elapsed_seconds > 1){
        previous_seconds = current_seconds;
        double fps = (double)frame_count / elapsed_seconds;
        char tmp[128];
        sprintf(tmp, "opengl @ fps: %.2f", fps);
        glfwSetWindowTitle(window, tmp);
        frame_count = 0;
    }
    frame_count++;
}


double get_fps(){
    static double previous_seconds = glfwGetTime();
    static int frame_count;
    double current_seconds = glfwGetTime();
    double elapsed_seconds = current_seconds - previous_seconds;
    static double fps;
    if(elapsed_seconds > 1){
        previous_seconds = current_seconds;
        fps = (double)frame_count / elapsed_seconds;
        frame_count = 0;
    }
    frame_count++;

    return fps;
}


struct bbox get_AABB(GLfloat* vbuffer, int n_vert){
    /* I procrastinated a bit and made this unnecessary schematic

                            (x_max, y_max, z_min)
                                |   (x_max, y_max, z_max)
                                |         |
                                v         v
                                __________    
                               /|        /|
                              / |       / |
    (x_min, y_max, z_min)--->/__|_____ /<-|---(x_min, y_max, z_max)
                            |   |     |   |
    (x_max, y_min, z_min)---|-->|_____|___| <---(x_max, y_min, z_max)
                            |  /      |  /
                            | /       | /
                            |/________|/
                            ^         ^
                            |         |
                            |   (x_min, y_min, z_max)
                            |
                         (x_min, y_min, z_min)

                              y|  / x
                               | /
                               |/____
                                z
    */
    GLfloat x_max = -std::numeric_limits<GLfloat>::max(), x_min = std::numeric_limits<GLfloat>::max(),
            y_max = -std::numeric_limits<GLfloat>::max(), y_min = std::numeric_limits<GLfloat>::max(),
            z_max = -std::numeric_limits<GLfloat>::max(), z_min = std::numeric_limits<GLfloat>::max();
    struct bbox da_box;

    for(int i = 0; i < n_vert; i++){
        if(vbuffer[i * 3] > x_max)
            x_max = vbuffer[i * 3];
        if(vbuffer[i * 3] < x_min)
            x_min = vbuffer[i * 3];

        if(vbuffer[i * 3 + 1] > y_max)
            y_max = vbuffer[i * 3 + 1];
        if(vbuffer[i * 3 + 1] < y_min)
            y_min = vbuffer[i * 3 + 1];

        if(vbuffer[i * 3 + 2] > z_max)
            z_max = vbuffer[i * 3 + 2];
        if(vbuffer[i * 3 + 2] < z_min)
            z_min = vbuffer[i * 3 + 2];
    }

    da_box.vert[0] = {x_max, y_max, z_min};
    da_box.vert[1] = {x_max, y_max, z_max};
    da_box.vert[2] = {x_min, y_max, z_min};
    da_box.vert[3] = {x_max, y_min, z_min};
    da_box.vert[4] = {x_min, y_max, z_max};
    da_box.vert[5] = {x_max, y_min, z_max};
    da_box.vert[6] = {x_min, y_min, z_max};
    da_box.vert[7] = {x_min, y_min, z_min};

    return da_box;
}


bool check_gl_errors(bool print, const char* caller){
    bool error = false;
    GLenum e = glGetError();

    while(e != GL_NO_ERROR){
        error = true;
        if(print){
            std::cerr << "check_gl_errors: GL error with value " << e << " from "
                      << caller << std::endl;
            log("check_gl_errors: GL error with value ", e, " from ", caller);
        }

        e = glGetError();
    }

    return error;
}
