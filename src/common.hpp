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

#define LOG_FILE "log.txt"


#endif
