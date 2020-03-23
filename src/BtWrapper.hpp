#ifndef BT_WRAPPER_HPP
#define BT_WRAPPER_HPP

#include <iostream>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "log.hpp"
#include "Input.hpp"
#include "WindowHandler.hpp"


class BtWrapper{
    private:
        btDefaultCollisionConfiguration* m_collisionConfiguration;
        btCollisionDispatcher* m_dispatcher;
        btBroadphaseInterface* m_overlappingPairCache;
        btSequentialImpulseConstraintSolver* m_solver;
        btDiscreteDynamicsWorld* m_dynamicsWorld;

        const WindowHandler* m_window_handler;
        const Input* m_input;
    public:
        BtWrapper();
        BtWrapper(const btVector3& gravity, const WindowHandler* window_handler, const Input* input);
        ~BtWrapper();

        void addRigidBody(btRigidBody* body);
        void deleteBody(btRigidBody* body);
        //void 

        void stepSimulation(btScalar time_step, int max_sub_steps);
        
};


#endif
