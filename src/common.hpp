#ifndef COMMON_HPP
#define COMMON_HPP
#include <map>
#include "BasePart.hpp"

typedef unsigned int EntityId;
typedef std::map<const std::string, BasePart *> partsMap;

#define UNUSED(expr) do { (void)(expr); } while (0)  //to avoid unused parameter warnings in callbacks

struct color{
    float r;
    float g;
    float b;
};

#define OPTION_INVALID -1
#define OPTION_EXIT 0
#define OPTION_APPEND_NODE 1
#define OPTION_DELETE_NODE 2
#define OPTION_CHANGE_NAME 3
#define OPTION_CHANGE_ROOT 4
#define OPTION_SHOW_PARTS 5
#define OPTION_PRINT_TREE 6
#define OPTION_PRINT_OPTIONS 7
#define LOG_GL_MAX_LENGTH 2048

//math defines
#define TAU 2.0 * M_PI
#define ONE_DEG_IN_RAD ( 2.0 * M_PI ) / 360.0 // 0.017444444
#define ONE_RAD_IN_DEG 360.0 / ( 2.0 * M_PI ) // 57.2957795

#define LOG_FILE "log.txt"


#endif
