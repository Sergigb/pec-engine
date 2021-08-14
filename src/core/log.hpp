#ifndef GL_INIT_HPP
#define GL_INIT_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>

#include "common.hpp"


/*
 * Starts the log file.
 */
int log_start();

/*
 * Logs some useful opengl stuff.
 */
void log_gl_params();

/*
 * Loging variadic function. Example usage:
 * log("You can print ints like this: ", 12, " or floats ", 5.9);
 */
template<typename T, typename... Args> int log(T, Args...);

/*
 * Loging function with a single argument.
 */
template<typename T> int log(T);

#include "log.tpp"  //template functions implementations

#endif