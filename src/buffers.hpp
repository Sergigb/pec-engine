#ifndef BUFFERS_HPP
#define BUFFERS_HPP

#include <vector>
#include <mutex>
#include <memory>

#include "maths_funcs.hpp"

class Object;

// buffers for synchronizing physics and rendering

struct object_transform{
    std::shared_ptr<Object> object_ptr;
    math::mat4 transform;
};

enum buffer_manager: char{none = 0, buffer_1 = 1, buffer_2 = 2};

struct render_buffers{
    std::vector<object_transform> buffer1;
    std::vector<object_transform> buffer2;
    std::mutex buffer1_lock;
    std::mutex buffer2_lock;
    buffer_manager last_updated;
};

/*
 * Command buffer messages
 */

class btVector3;
class btQuaternion;
class BasePart;
class btTypedConstraint;

struct set_motion_state_msg{
    Object* object;
    btVector3 origin;
    btQuaternion initial_rotation;
};


struct add_contraint_msg{
    BasePart* part;
    std::unique_ptr<btTypedConstraint> constraint_uptr;
};


struct add_body_msg{
    BasePart* part;
    btVector3 origin;
    btVector3 inertia;
    btQuaternion rotation;
};


#endif