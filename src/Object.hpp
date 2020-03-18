#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <algorithm>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "Model.hpp"
#include "maths_funcs.hpp"
#include "BtWrapper.hpp"

/* Simple class for rigid objects, contains a rigid body object and its model*/

class Object{
    private:
        Model* m_model;
        btRigidBody* m_body;
        BtWrapper* m_bt_wrapper;
        math::vec3 m_mesh_color;
    public:
        Object(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation, btScalar mass);
        ~Object();

        void setColor(math::vec3 color);
        int render();
};


#endif
