#ifndef BUFFERS_HPP
#define BUFFERS_HPP

#include <vector>
#include <mutex>
#include <memory>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "maths_funcs.hpp"

class Object;
class Planet;


/******************/
/*                */
/* Render buffers */
/*                */
/******************/

/*
 * Object transform struct used in the render buffers, holds a shared pointer of an Object and
 * its transform. The reason behind using shared pointers is that the rendering thread runs
 * independently from the logic and physics thread. By using shared pointers we avoid accessing 
 * from the rendering thread Objects that may have been deleted by the logic thread. Semantically
 * speaking, these buffers don't own the Objects they point to, these objects are always owned by
 * the AssetManager. If an Object pointer is reseted in the logic thread, it might still be 
 * accessible from one or both of the buffers. As soon as the buffer or buffers are cleared, the
 * object is finally destroyed.
 */

struct object_transform{
    std::shared_ptr<Object> object_ptr;
    math::mat4 transform;

    /*
     * Constructor
     * 
     * @ptr: rvalue reference to the shared pointer of the object, we use std::move to make it go
     * fast as shit. It takes ownership of the passed pointer.
     * @t: constant reference to a single-precision transform matrix, needs to be relative to the
     * centered camera or it will lose precision.
     */
    object_transform(std::shared_ptr<Object>&& ptr, const math::mat4& t){
        object_ptr = std::move(ptr);
        transform = t;
    }
};


/*
 * Similarly to object_transform, this struct is used to render Planets. It also holds a transform,
 * but in this case we use raw pointers because planets don't get destroyed while the simulation is
 * on.
 */
struct planet_transform{
    Planet* planet_ptr; // maybe could be constant?
    dmath::mat4 transform;

    /*
     * Constructor
     *
     * @ptr: raw pointer to the planet object.
     * @t: constant reference to a double-precision transform matrix, should hold the global 
     * transform of the object.
     */
    planet_transform(Planet* ptr, const dmath::mat4& t){
        planet_ptr = ptr;
        transform = t;
    }
};


/* Buffer manager, essentialy an enum that helps manage the latest updated buffer */
enum buffer_manager: char{none = 0, buffer_1 = 1, buffer_2 = 2};

/*
 * Render buffer struct, holds the necessary stuff used to render the scene.
 *
 * @buffer: vector of object_transforms.
 * @planet_buffer: vector of planet transforms.
 * @view_mat: state of the view matrix in the current tick.
 * @cam_origin: origin of the camera at that tick, used to render Planets.
 * @buffer_lock: lock of the buffer, used to avoid the AssetManager and the render thread 
 * manipulating the buffers at the same time (something rare but can happen, see 
 * AssetManager::updateBuffers or RenderContext::renderSceneEditor for example.
 */
struct render_buffer{
    std::vector<object_transform> buffer;
    std::vector<planet_transform> planet_buffer;
    math::mat4 view_mat;
    dmath::vec3 cam_origin;
    std::mutex buffer_lock;
};


/*
 * This struct holds two (render_buffer)s and a buffer manager. The buffer manager indicates the
 * lastest updated buffer. The render thread will always use the last updated buffer to render,
 * while the asset manager will always update the other one. This avoids waiting for the buffer
 * to be free, but it can still happen that both threads want to use the same buffer (again see
 * AssetManager::updateBuffers). In that case the locks avoid writing/reading races.
 *
 * @buffer_1: buffer 1.
 * @buffer_2: buffer 2.
 * @last_updated: indicates the last updated buffer.
 */
struct render_buffers{
    render_buffer buffer_1;
    render_buffer buffer_2;
    buffer_manager last_updated;
};


/*************************************************************************************************/
/*                                                                                               */
/*                                    Command buffer messages                                    */
/*                                                                                               */
/* These messages are used to communicate Vessels and Object/Parts with the asset manager. These */
/* messages are stored in buffers in the AssetManager, some of which be accessed by Parts and    */
/* Vessels via the AssetManagerInterface.                                                        */
/*                                                                                               */
/* We don't explain how the buffers should be used in here, for that see the header file of      */
/* AssetManagerInterface or AssetManager.                                                        */
/*************************************************************************************************/

class BasePart;


/* Message used in m_set_motion_state_buffer, can't be accessed via AssetManagerInterface */
struct set_motion_state_msg{
    Object* object;
    btVector3 origin;
    btQuaternion initial_rotation;

    /* Constructor.
     *
     * @obj: raw pointer to the object.
     * @orig: constant reference to the new origin of the object.
     * @rot: constant reference to the new rotation of the object.
     */
    set_motion_state_msg(Object* obj, const btVector3& orig, const btQuaternion& rot){
        object = obj;
        origin = orig;
        initial_rotation = rot;
    }
};


/* Message used in m_add_constraint_buffer, can be accessed via AssetManagerInterface::addConstraint */
struct add_contraint_msg{
    BasePart* part;
    std::unique_ptr<btTypedConstraint> constraint_uptr;

    /* Constructor.
     *
     * @ptr: raw pointer to the object.
     * @c_uptr: rvalue reference to the unique pointer of the constraint, this message will take 
     * ownership of the object.
     */
    add_contraint_msg(BasePart* ptr, std::unique_ptr<btTypedConstraint>&& c_uptr){
        part = ptr;
        constraint_uptr = std::move(c_uptr);
    }
};


/* Message used in m_add_body_buffer, can be accessed via AssetManagerInterface::addBody */
struct add_body_msg{
    BasePart* part;
    btVector3 origin;
    btVector3 inertia;
    btQuaternion rotation;

    /* Constructor.
     *
     * @ptr: raw pointer to the object that contains the rigid body.
     * @orig: constant reference to the origin of the object.
     * @iner: constant reference to the inertia vector of the object.
     * @rot: constant reference to the rotation of the object.
     */
    add_body_msg(BasePart* ptr, const btVector3& orig, const btVector3& iner, const btQuaternion& rot){
        part = ptr;
        origin = orig;
        inertia = iner;
        rotation = rot;
    }
};


/* Message used in m_apply_force_buffer, can be accessed via AssetManagerInterface::applyForce */
struct apply_force_msg{
    BasePart* part;
    btVector3 force;
    btVector3 rel_pos;

    /* Constructor.
     *
     * @ptr: raw pointer to the object that contains the rigid body.
     * @f: constant reference of the force vector we want to apply, in newtons.
     * @r_pos: constant reference of the relative application point of the force, in meters.
     */
    apply_force_msg(BasePart* ptr, const btVector3& f, const btVector3& r_pos){
        part = ptr;
        force = f;
        rel_pos = r_pos;
    }
};


/* Message used in m_set_mass_buffer, can be accessed via AssetManagerInterface::setMassProps */
struct set_mass_props_msg{
    BasePart* part;
    double mass;

    /* Constructor.
     *
     * @ptr: raw pointer to the object that contains the rigid body.
     * @m: new mass value, in kilograms.
     */
    set_mass_props_msg(BasePart* ptr, double m){
        part = ptr;
        mass = m;
    }
};


#endif