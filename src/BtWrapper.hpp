#ifndef BT_WRAPPER_HPP
#define BT_WRAPPER_HPP

#include <iostream>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "log.hpp"
#include "maths_funcs.hpp"


class Object;

class BtWrapper{
    private:
        btDefaultCollisionConfiguration* m_collisionConfiguration;
        btCollisionDispatcher* m_dispatcher;
        btBroadphaseInterface* m_overlappingPairCache;
        btSequentialImpulseConstraintSolver* m_solver;
        btDiscreteDynamicsWorld* m_dynamicsWorld;

    public:
        BtWrapper();
        BtWrapper(const btVector3& gravity);
        ~BtWrapper();

        void addRigidBody(btRigidBody* body);
        void deleteBody(btRigidBody* body);
        Object* testMousePick(float fb_width, float fb_heigth, float mouse_x, float mouse_y, const math::mat4& proj_matrix, const math::mat4& view_matrix, double dist);

        void stepSimulation(btScalar time_step, int max_sub_steps);
        
};


#endif
