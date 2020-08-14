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
#include "multithreading.hpp"


// bullet collision groups (for now, only filtering rays)
#define CG_OBJECT 1
#define CG_KINEMATIC 2
#define CG_PART 4
#define CG_RAY 8
#define CG_RAY_EDITOR_RADIAL 16 // editor radial attaching


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
        void noticeLogic();
        void waitLogic();

        std::thread m_thread_simulation;
        bool m_simulation_paused, m_end_simulation;
        double m_average_load;
        struct thread_monitor* m_thread_monitor;

        // synchronization
        struct render_buffers* m_buffers;
    public:
        BtWrapper();
        BtWrapper(const btVector3& gravity, render_buffers* buff_manager, thread_monitor* thread_monitor);
        ~BtWrapper();

        void addRigidBody(btRigidBody* body, short group, short mask);
        void addConstraint(btTypedConstraint *constraint, bool disable_collision_between_bodies);
        void removeConstraint(btTypedConstraint *constraint);
        void removeBody(btRigidBody* body);
        Object* testRay(const math::vec3& ray_start_world, const math::vec3& ray_end_world) const;
        Object* testRay(btCollisionWorld::ClosestRayResultCallback& ray_callback, const btVector3& ray_start, const btVector3& ray_end) const;
        void updateCollisionWorldSingleAABB(btRigidBody* body);
        double getAverageLoadTime() const;

        void startSimulation(btScalar time_step, int max_sub_steps);
        void stopSimulation();
        void pauseSimulation(bool stop_simulation);
};


#endif
