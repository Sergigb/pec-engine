#ifndef BUFFERS_HPP
#define BUFFERS_HPP

#include <vector>
#include <mutex>
#include <memory>

#include "maths_funcs.hpp"

class Object;
class Planet;

// buffers for synchronizing physics and rendering

struct object_transform{
    std::shared_ptr<Object> object_ptr;
    math::mat4 transform;

    object_transform(const std::shared_ptr<Object>& ptr, math::mat4& t){
        object_ptr = std::move(ptr);
        transform = t;
    }
};


struct planet_transform{
    Planet* planet_ptr;
    dmath::mat4 transform;

    planet_transform(Planet* ptr, dmath::mat4& t){
        planet_ptr = ptr;
        transform = t;
    }
};


enum buffer_manager: char{none = 0, buffer_1 = 1, buffer_2 = 2};

struct render_buffers{
    std::vector<object_transform> buffer1;
    std::vector<object_transform> buffer2;
    std::vector<planet_transform> planet_buffer1;
    std::vector<planet_transform> planet_buffer2;
    math::mat4 view_mat1;
    math::mat4 view_mat2;
    dmath::vec3 cam_origin1;
    dmath::vec3 cam_origin2;
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

    set_motion_state_msg(Object* obj, const btVector3& orig, const btQuaternion& rot){
        object = obj;
        origin = orig;
        initial_rotation = rot;
    }
};


struct add_contraint_msg{
    BasePart* part;
    std::unique_ptr<btTypedConstraint> constraint_uptr;

    add_contraint_msg(BasePart* ptr, std::unique_ptr<btTypedConstraint>& c_uptr){
        part = ptr;
        constraint_uptr = std::move(c_uptr);
    }
};


struct add_body_msg{
    BasePart* part;
    btVector3 origin;
    btVector3 inertia;
    btQuaternion rotation;

    add_body_msg(BasePart* ptr, const btVector3& orig, const btVector3& iner, const btQuaternion& rot){
        part = ptr;
        origin = orig;
        inertia = iner;
        rotation = rot;
    }
};


struct apply_force_msg{
    BasePart* part;
    btVector3 force;
    btVector3 rel_pos;

    apply_force_msg(BasePart* ptr, const btVector3& f, const btVector3& r_pos){
        part = ptr;
        force = f;
        rel_pos = r_pos;
    }
};


struct set_mass_props_msg{
    BasePart* part;
    double mass;

    set_mass_props_msg(BasePart* ptr, double m){
        part = ptr;
        mass = m;
    }
};


#endif