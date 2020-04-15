#ifndef BT_WRAPPER_HPP
#define BT_WRAPPER_HPP

#include <iostream>
#include <thread>
#include <iomanip>
#include <vector>
#include <mutex>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "log.hpp"
#include "maths_funcs.hpp"
#include "common.hpp"


class Object;

class BtWrapper{
    private:
        using buffer = std::vector<object_transform>;

        btDefaultCollisionConfiguration* m_collisionConfiguration;
        btCollisionDispatcher* m_dispatcher;
        btBroadphaseInterface* m_overlappingPairCache;
        btSequentialImpulseConstraintSolver* m_solver;
        btDiscreteDynamicsWorld* m_dynamicsWorld;

        void runSimulation(btScalar time_step, int max_sub_steps);
        void updateBuffers();
        void updateBuffer(buffer* buffer_);

        std::thread m_thread_simulation;
        bool m_simulation_paused, m_end_simulation;
        double m_average_load, m_average_sleep;
        std::chrono::duration<double, std::milli> m_elapsed_time;

        // synchronization
        buffer* m_buffer1;
        buffer* m_buffer2;
        std::mutex* m_buffer1_lock;
        std::mutex* m_buffer2_lock;
        std::mutex* m_manager_lock;
        buffer_manager* m_last_updated;
    public:
        BtWrapper();
        BtWrapper(const btVector3& gravity, buffer* buffer1, buffer* buffer2, std::mutex* buff1_lock,
                  std::mutex* buff2_lock, std::mutex* manager_lock, buffer_manager* manager);
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
