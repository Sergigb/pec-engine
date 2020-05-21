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
    protected:
        Model* m_model;
        BtWrapper* m_bt_wrapper;
        
        std::unique_ptr<btMotionState> m_motion_state;
        math::vec3 m_mesh_color;
        math::mat4 m_mesh_transform;
        bool m_has_transform;
    public:
        std::unique_ptr<btRigidBody> m_body; // made public for convenience

        Object(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation, btScalar mass);
        Object();
        Object(const Object& obj);
        ~Object();

        math::mat4 getRigidBodyTransformSingle() const;
        void getRigidBodyTransformDouble(double* mat4) const;

        void applyCentralForce(const btVector3& force);
        void applyTorque(const btVector3& torque);
        void setColor(math::vec3 color);
        void setMeshScale(float scale);
        void setMeshScale(const math::vec3& scale);
        void setMeshTransform(const math::mat4& transform);
        void setMotionState(const btVector3& origin, const btQuaternion& initial_rotation);
        void activate(bool activate);
        int render();
        int render(math::mat4 body_transform);
};


#endif
