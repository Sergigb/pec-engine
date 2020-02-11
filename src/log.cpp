 #include "log.hpp"



int log_start(){
    std::chrono::system_clock::time_point now;
    std::time_t now_tt;
    std::ofstream logfile;

    now = std::chrono::system_clock::now();
    now_tt = std::chrono::system_clock::to_time_t(now);

    logfile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
    try{
        std::cout << "Starting log file" << std::endl;
        logfile.open(LOG_FILE);
        logfile << "Starting log file. Local time: " << std::ctime(&now_tt);
        logfile.close();    
    }
    catch(const std::ios_base::failure& e){
        std::cerr << "Error starting log file: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}


void log_gl_params(){
    std::ostringstream message;
    GLenum params[] = {
        GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
        GL_MAX_CUBE_MAP_TEXTURE_SIZE,
        GL_MAX_DRAW_BUFFERS,
        GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
        GL_MAX_LIGHTS,
        GL_MAX_TEXTURE_IMAGE_UNITS,
        GL_MAX_TEXTURE_SIZE,
        GL_MAX_VARYING_FLOATS,
        GL_MAX_VERTEX_ATTRIBS,
        GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
        GL_MAX_VERTEX_UNIFORM_COMPONENTS,
        GL_MAX_VIEWPORT_DIMS,
        GL_STEREO,
    };
    const char *names[] = {
        "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS",
        "GL_MAX_CUBE_MAP_TEXTURE_SIZE",
        "GL_MAX_DRAW_BUFFERS",
        "GL_MAX_FRAGMENT_UNIFORM_COMPONENTS",
        "GL_MAX_LIGHTS",
        "GL_MAX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_TEXTURE_SIZE",
        "GL_MAX_VARYING_FLOATS",
        "GL_MAX_VERTEX_ATTRIBS",
        "GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS",
        "GL_MAX_VERTEX_UNIFORM_COMPONENTS",
        "GL_MAX_VIEWPORT_DIMS",
        "GL_STEREO",
    };

    message << "GL Context Params:";
    for(int i = 0; i < 11; i++){
        int v = 0;
        glGetIntegerv(params[i], &v);
        message << std::endl;
        message << '\t' << names[i] << ": " << v;
    }
    log(message.str().c_str());
    // others
   /* int v[2];
    v[0] = v[1] = 0;
    glGetIntegerv(params[10], v);
    log(names[10], v[0], v[1]);
    unsigned char s = 0;
    glGetBooleanv(params[12], &s);
    log(names[12], (unsigned int)s);
    log("-----------------------------\n");*/
}
