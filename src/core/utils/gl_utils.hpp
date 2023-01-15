#ifndef GL_UTILS_HPP
#define GL_UTILS_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

#include "../maths_funcs.hpp"


struct GLFWwindow;

/*
 * Logs the information of an existing shader.
 *
 * @shader_index: index of the shader.
 */
void log_shader_info(GLuint shader_index);

/*
 * Logs the information of an existing programme.
 *
 * @program_index: index of the program.
 */
void log_programme_info(GLuint program_index);

/*
 * Validates the state of an existing programme.
 *
 * @program_index: index of the program.
 */
int log_validate_programme(GLuint program_index);

/*
 * Creates a shader given a path to a shader file and the type of shader.
 *
 * @filename: path to the shader file.
 * @shader: reference to GLuint where the shader index is going to be stored.
 * @type: shader type, most likely GL_VERTEX_SHADER or GL_FRAGMENT_SHADER, check the OpenGL 
 * reference pages for more shader types:
 * registry.khronos.org/OpenGL-Refpages/gl4/html/glCreateShader.xhtml
 */
int create_shader(const char* filename, GLuint &shader, GLenum type);

/*
 * Creates a shader programme from a vertex hsader and a fragment shader. The two shaders get 
 * destroyed after creating the programme.
 *
 * @vert: index of the vertex shader.
 * @frag: index of the fragment shader.
 * @programme_index: reference to GLuint where the index of the new programme is going to be 
 * stored.
 */
int create_programme(GLuint vert, GLuint frag, GLuint &programme_index);

/*
 * Wrapper function around create_programme and create_shader, in practice you should most likely
 * only use this function. Returns the index of the created programme.
 *
 * @vert_file_name: path to the vertex shader file.
 * @frag_file_name: path to the fragment shader file.
 */
GLuint create_programme_from_files(const char* vert_file_name, const char* frag_file_name);

/*
 * Loads a 3D scene from a file. This function creates a vao object that contains the different 
 * buffers if the 3D file.
 *
 * @pFile: path to the 3D file.
 * @vao: reference to GLuint of the vao object for this file, it will contain the different buffers
 * that the file may contain. This function triangulates vertices so it will always use indexed 
 * geometry, so when we want to render this object we should use glDrawElements.
 * @point_cout: reference to int, will contain the number of vertices of the mesh, without taking
 * into account the indices (so the real number of vertices is point_cout * 3).
 * @cs_radius: reference to float, will contain the radius of the bounding sphere of this object.
 * Useful for culling.
 * @aabb: reference to a bounding box struct (defined in Model.hpp) which will cointain the 
 * axis-aligned bouding box of the mesh.
 */
int load_scene(const std::string pFile, GLuint& vao, int& point_cout, float& cs_radius, struct bbox& aabb);

/*
 * Old function to display the FPS in the name of the window, not used.
 *
 * @window: raw pointer to a GLFWwindow object.
 */
void update_fps_counter(GLFWwindow* window);

/*
 * Returns the FPS, it should be only called once per tick. I would say don't use it because it 
 * uses static variables.
 */
double get_fps();

/*
 * Returns a 3D axis-aligned bounding box for a given mesh. The bounding box struct is defined 
 * at the end of this file.
 *
 * @vbuffer: vertex buffer array of type GLfloat, should be an array of 3 elements per vertex.
 * @n_vert: vertex count.
 */
struct bbox get_AABB(GLfloat* vbuffer, int n_vert);

/*
 * Unimplemented function, I don't remember what I wanted to do with it. I think it's supposed to
 * create an oriented bounding box, but it's probably difficult to compute that. Ideally we want
 * the smallest bouding box that encloses a mesh, but it probably requires some sort of iterative
 * algorithm that is probably not worth the trouble, so fuck that man.
 */
struct bbox get_OBB(GLfloat* vbuffer, int n_vert); // todo

/*
 * Checks if there are OpenGL errors, it basically calls glGetError.
 *
 * @print: if true it prints (AND LOGS!!?) the error. Check the error codes at your nearest OpenGL
 * reference page.
 * @caller: optional argument, you can pass the name of your function (or whatever you want 
 * really!) and it will print it jointly with the error.
 */
bool check_gl_errors(bool print, const char* caller = "unknown caller");

struct bbox{
    math::vec3 vert[8];
};

#endif
