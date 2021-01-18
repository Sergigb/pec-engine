#ifndef BT_WRAPPER_HPP
#define BT_WRAPPER_HPP

#include <memory>
#include <thread>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "maths_funcs.hpp"


// bullet collision groups
#define CG_DEFAULT 1
#define CG_OBJECT 2
#define CG_KINEMATIC 4
#define CG_PART 8
#define CG_RAY 16
#define CG_RAY_EDITOR_SELECT 32
#define CG_RAY_EDITOR_RADIAL 64 // editor radial attaching


class Object;

struct thread_monitor;

/*
#include "BasePart.hpp"
struct myFilterCallback: public btOverlapFilterCallback{
    virtual bool needBroadphaseCollision(btBroadphaseProxy*proxy0,btBroadphaseProxy*proxy1) const{
        BasePart* a;
        BasePart* b;
        a = dynamic_cast<BasePart*>(static_cast<btCollisionObject*>(proxy0->m_clientObject)->getUserPointer());
        b = dynamic_cast<BasePart*>(static_cast<btCollisionObject*>(proxy1->m_clientObject)->getUserPointer());

        if(a && b){
            std::string name;
            a->getName(name);
            std::cout << name << std::endl;
        }

        bool collides = proxy0->m_collisionFilterGroup && proxy1->m_collisionFilterGroup;
        return collides;
    }
};
*/

struct iv_array{
    std::unique_ptr<btTriangleIndexVertexArray> bt_ivarray;
    std::unique_ptr<btScalar[]> points;
    std::unique_ptr<int[]> indices;
};


class Physics{
    private:
        std::unique_ptr<btDefaultCollisionConfiguration> m_collision_configuration;
        std::unique_ptr<btCollisionDispatcher> m_dispatcher;
        std::unique_ptr<btBroadphaseInterface> m_overlapping_pair_cache;
        std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
        std::unique_ptr<btDiscreteDynamicsWorld> m_dynamics_world;

        void init(const btVector3& gravity);

        void runSimulation(btScalar time_step, int max_sub_steps);
        void noticeLogic();
        void waitLogic();

        void applyGravity();

        std::thread m_thread_simulation;
        bool m_simulation_paused, m_end_simulation;
        double m_average_load;
        struct thread_monitor* m_thread_monitor;
    public:
        Physics();
        Physics(const btVector3& gravity, thread_monitor* thread_monitor);
        ~Physics();

        void addRigidBody(btRigidBody* body, short group, short mask);
        void addConstraint(btTypedConstraint *constraint, bool disable_collision_between_bodies);
        void removeConstraint(btTypedConstraint *constraint);
        void removeBody(btRigidBody* body);
        Object* testRay(const math::vec3& ray_start_world, const math::vec3& ray_end_world) const;
        Object* testRay(btCollisionWorld::ClosestRayResultCallback& ray_callback, const btVector3& ray_start, const btVector3& ray_end) const;
        void updateCollisionWorldSingleAABB(btRigidBody* body);
        double getAverageLoadTime() const;
        const btDiscreteDynamicsWorld* getDynamicsWorld() const;
        btDiscreteDynamicsWorld* getDynamicsWorld();

        void startSimulation(btScalar time_step, int max_sub_steps);
        void stopSimulation();
        void pauseSimulation(bool stop_simulation);
};


#endif
