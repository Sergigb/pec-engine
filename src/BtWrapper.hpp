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
        void addConstraint(btTypedConstraint *constraint, bool disable_collision_between_bodies);
        void deleteBody(btRigidBody* body);
        Object* testRay(const math::vec3& ray_start_world, const math::vec3& ray_end_world) const;
        void updateCollisionWorldSingleAABB(btRigidBody* body);

        void stepSimulation(btScalar time_step, int max_sub_steps);
        
};


#endif
