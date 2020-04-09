#ifndef BT_WRAPPER_HPP
#define BT_WRAPPER_HPP

#include <iostream>
#include <memory>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "log.hpp"
#include "maths_funcs.hpp"


class Object;

class BtWrapper{
    private:
        std::unique_ptr<btDefaultCollisionConfiguration> m_collision_configuration;
        std::unique_ptr<btCollisionDispatcher> m_dispatcher;
        std::unique_ptr<btBroadphaseInterface> m_overlapping_pair_cache;
        std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
        std::unique_ptr<btDiscreteDynamicsWorld> m_dynamics_world;

        void init(const btVector3& gravity);
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
