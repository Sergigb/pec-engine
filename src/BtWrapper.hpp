#ifndef BT_WRAPPER_HPP
#define BT_WRAPPER_HPP

#include <iostream>
#include <memory>
#include <thread>
#include <iomanip>
#include <vector>
#include <mutex>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "log.hpp"
#include "maths_funcs.hpp"
#include "buffers.hpp"


class Object;

class BtWrapper{
    private:
        std::unique_ptr<btDefaultCollisionConfiguration> m_collision_configuration;
        std::unique_ptr<btCollisionDispatcher> m_dispatcher;
        std::unique_ptr<btBroadphaseInterface> m_overlapping_pair_cache;
        std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
        std::unique_ptr<btDiscreteDynamicsWorld> m_dynamics_world;

        void init(const btVector3& gravity);

        void runSimulation(btScalar time_step, int max_sub_steps);
        void updateBuffers();
        void updateBuffer(std::vector<object_transform>* buffer_);

        std::thread m_thread_simulation;
        bool m_simulation_paused, m_end_simulation;
        double m_average_load, m_average_sleep;
        std::chrono::duration<double, std::micro> m_elapsed_time;

        // synchronization
        struct render_buffers* m_buffers;
    public:
        BtWrapper();
        BtWrapper(const btVector3& gravity, render_buffers* buff_manager);
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
