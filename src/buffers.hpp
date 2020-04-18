#ifndef BUFFERS_HPP
#define BUFFERS_HPP

#include <vector>
#include <mutex>

#include "maths_funcs.hpp"

class Object;

// buffers for synchronizing physics and rendering

struct object_transform{
    Object* object_ptr;
    math::mat4 transform;
};

enum buffer_manager: char{none = 0, buffer_1 = 1, buffer_2 = 2};

struct trans_dbl_buffers{
    std::vector<object_transform> buffer1;
    std::vector<object_transform> buffer2;
    std::mutex buffer1_lock;
    std::mutex buffer2_lock;
    buffer_manager last_updated;
};

#endif