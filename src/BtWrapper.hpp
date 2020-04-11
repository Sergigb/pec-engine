#ifndef BT_WRAPPER_HPP
#define BT_WRAPPER_HPP

#include <iostream>
#include <thread>
#include <iomanip>

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

        void runSimulation(btScalar time_step, int max_sub_steps);

        std::thread m_thread_simulation;
        bool m_simulation_paused, m_end_simulation;
        double m_average_load, m_average_sleep;
        std::chrono::duration<double, std::milli> m_simulation_time;
    public:
        BtWrapper();
        BtWrapper(const btVector3& gravity);
        ~BtWrapper();

        void addRigidBody(btRigidBody* body);
        void addConstraint(btTypedConstraint *constraint, bool disable_collision_between_bodies);
        void deleteBody(btRigidBody* body);
        Object* testRay(const math::vec3& ray_start_world, const math::vec3& ray_end_world) const;
        void updateCollisionWorldSingleAABB(btRigidBody* body);
        double getAverageLoadTime() const;
        double getAverageSleepTime() const;

        void startSimulation(btScalar time_step, int max_sub_steps);
        void stopSimulation();
        void pauseSimulation(bool stop_simulation);
};


#endif
