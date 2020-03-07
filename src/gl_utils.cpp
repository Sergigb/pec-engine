#include "gl_utils.hpp"


void glfw_error_callback(int error, const char* description){
	std::cout << "GLFW error " << error << "(" << description << ")" << std::endl;
	log("GLFW error number ", error, " (", description, ")");
}


int init_gl(GLFWwindow** g_window, int gl_width, int gl_height){
    log("Starting GLFW ", glfwGetVersionString());

    glfwSetErrorCallback(glfw_error_callback);
    if(!glfwInit()){
        std::cerr << "ERROR: could not start GLFW3, check the log" << std::endl;
        log("ERROR: could not start GLFW3");
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
//    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /*GLFWmonitor* mon = glfwGetPrimaryMonitor ();
    const GLFWvidmode* vmode = glfwGetVideoMode (mon);
    g_window = glfwCreateWindow (
                    vmode->width, vmode->height, "Extended GL Init", mon, NULL
);*/

    *g_window = glfwCreateWindow(gl_width, gl_height, "The window with no name", NULL, NULL);
    if(!*g_window){
        std::cerr << "ERROR: could not open window with GLFW3, check the log" << std::endl;
        log("ERROR: could not open window with GLFW3");
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(*g_window);

    glfwWindowHint(GLFW_SAMPLES, 4); //x4 MSAA, we should add a function to change it

    log("Starting GLEW");
    glewExperimental = GL_TRUE;
    glewInit();

    // get version info
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "Renderer: " << renderer << std::endl;
    std::cout << "OpenGL version supported: " << version << std::endl;
    log("Renderer: ", renderer, ", using OpenGL version: ", version);

    // general gl setup
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glfwSwapInterval(0);

    return EXIT_SUCCESS;
}


void log_shader_info(GLuint shader_index){
    int actual_length = 0;
    char message[LOG_GL_MAX_LENGTH];
    glGetShaderInfoLog(shader_index, LOG_GL_MAX_LENGTH, &actual_length, message);
    //std::cout << "Shader info log for shader with index " << shader_index << ": " << message << std::endl;
    log("Shader info log for shader with index ", shader_index, ": ", message);
}


void log_programme_info(GLuint program_index){
    int actual_length = 0;
    char message[LOG_GL_MAX_LENGTH];
    glGetProgramInfoLog(program_index, LOG_GL_MAX_LENGTH, &actual_length, message);
    //std::cout << "Program info log for program with index index " << program_index << ": " << message << std::endl;
    log("Program info log for program with index index ", program_index, ": ", message);
}


int log_validate_programme(GLuint program_index){
    GLint params = -1;
    glValidateProgram(program_index);
    glGetProgramiv(program_index, GL_VALIDATE_STATUS, &params);
    if(GL_TRUE != params){
        log("Program ", program_index, " GL_VALIDATE_STATUS = GL_FALSE, check the log");
        log_programme_info(program_index);
        return EXIT_FAILURE;
    }
    log("Program ", program_index, " GL_VALIDATE_STATUS = GL_TRUE");
    return EXIT_SUCCESS;
}


int create_shader(const char* filename, GLuint &shader, GLenum type){
    std::string shader_string;
    const GLchar* p;
    int params = -1;

    log("Creating shader from file ", filename);

    file_to_str(filename, shader_string);
    shader = glCreateShader(type);
    p = (const GLchar*)shader_string.c_str();
    glShaderSource(shader, 1, &p, NULL);
    glCompileShader(shader);

    // check for compile errors
    glGetShaderiv(shader, GL_COMPILE_STATUS, &params);
    if(params != GL_TRUE){
        log("ERROR: GL shader index ", shader, " did not compile (file ", filename, ")");
        std::cerr << "ERROR: GL shader index " << shader << " did not compile (file " << filename << ")" << std::endl;
        log_shader_info(shader);
        return EXIT_FAILURE;
    }
    log("Shader with index ", shader, " compiled (file ", filename, ")");
    return EXIT_SUCCESS;
}


int create_programme(GLuint vert, GLuint frag, GLuint &programme_index){
    GLint params = -1;

    programme_index = glCreateProgram();
    log("Created programme ", programme_index, ", attaching shaders ", vert, " and ", frag);
    glAttachShader(programme_index, vert);
    glAttachShader(programme_index, frag);
    glLinkProgram(programme_index);
    
    glGetProgramiv(programme_index, GL_LINK_STATUS, &params);
    if(GL_TRUE != params){
        log("ERROR: could not link shader programme GL index ", programme_index);
        std::cerr << "ERROR: GL programme index " << programme_index << " did not compile" << std::endl;
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
        log("Could not open file ", pFile, " (", importer.GetErrorString(), ")");
        std::cerr << "Could not open file " << pFile << " (" << importer.GetErrorString() << ")" << std::endl;
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