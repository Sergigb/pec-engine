#ifndef GL_UTILS_HPP
#define GL_UTILS_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "log.hpp"
#include "common.hpp"
#include "utils.hpp"


int init_gl(GLFWwindow** g_window, int gl_width, int gl_height);
void log_shader_info(GLuint shader_index);
void log_programme_info(GLuint program_index);
int log_validate_programme(GLuint program_index);
int create_shader(const char* filename, GLuint &shader, GLenum type);
int create_programme(GLuint vert, GLuint frag, GLuint &programme_index);
GLuint create_programme_from_files(const char* vert_file_name, const char* frag_file_name);
int load_scene(const std::string pFile, GLuint& vao, int& point_cout, float& cs_radius);
void update_fps_counter(GLFWwindow* window);


#endif