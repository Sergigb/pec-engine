#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <algorithm>
#include <memory>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "Model.hpp"
#include "maths_funcs.hpp"
#include "BtWrapper.hpp"

/* Simple class for rigid objects, contains a rigid body object and its model*/

class Object{
    private:
        Model* m_model;
        BtWrapper* m_bt_wrapper;
        
        std::unique_ptr<btRigidBody> m_body;
        std::unique_ptr<btMotionState> m_motion_state;
        math::vec3 m_mesh_color;
        math::mat4 m_scale_transform;
        float m_scale;
    public:
        Object(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation, btScalar mass);
        ~Object();

        btRigidBody* getRigidBody();

        void applyCentralForce(const btVector3& force);
        void applyTorque(const btVector3& torque);
        void setColor(math::vec3 color);
        void setMeshScale(float scale);
        void setMotionState(const btVector3& origin, const btQuaternion& initial_rotation);
        void activate(bool activate);
        int render();
};


#endif
