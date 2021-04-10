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


int log_start();
void log_gl_params();
template<typename T, typename... Args> int log(T, Args...);
template<typename T> int log(T);

#include "log.tpp"  //template functions implementations

#endif