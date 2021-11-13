#ifndef BT_WRAPPER_HPP
#define BT_WRAPPER_HPP

#include <memory>
#include <thread>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "maths_funcs.hpp"


/* custom bullet collision groups */
#define CG_DEFAULT 1
#define CG_OBJECT 2
#define CG_KINEMATIC 4
#define CG_PART 8
#define CG_RAY 16
#define CG_RAY_EDITOR_SELECT 32
#define CG_RAY_EDITOR_RADIAL 64 // editor radial attaching

#define GRAVITATIONAL_CONSTANT 6.67430e-11

/* time macros */
#define SECS_FROM_UNIX_TO_J2000 946684800.0
#define SECONDS_IN_A_CENTURY 3155760000.0
#define REAL_TIME_S 1./60.

/* distance macros */
#define AU_TO_METERS 149597900000.0

class Object;
class BaseApp;

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

/*
 * Index-vertex array, used to create the collision shape for btGImpactMeshShape using trimeshes
 * (a collison object shape composed by triangles that can move, used for terrain). See
 * load_bullet_trimesh in core/utils/assets_utils.cpp
 *
 * @bt_ivarray: Bullet's indexed triangle vertex array.
 * @points: array of doubles that represent the coordinates of the triangles, the stride and other
 * details are internally stored in bt_ivarray. This array is owned by this struct, it is not stored
 * in bt_ivarray so it's not a buffer.
 * @indices: array of indices, used to index the geometry. Like the point array, it is owned by
 * this struct.
 */
struct iv_array{
    std::unique_ptr<btTriangleIndexVertexArray> bt_ivarray;
    std::unique_ptr<btScalar[]> points;
    std::unique_ptr<int[]> indices;
};

/*
 * Struct with the different average load times of the physics thread.
 *
 * @thread_load: average load time of the thread (the update method)
 * @orbital_update: average load time of the orbital update
 * @apply_gravity: average load time of applying gravity
 * @step_bullet: average load time of bullet
 */
struct load_times{
    double thread_load, orbital_update, apply_gravity, step_bullet;
    double acc_thread_load, acc_orbital_update, acc_apply_gravity, acc_step_bullet;
};


/*
 * This class manages Bullet and applies gravity to the objects, among other things. The method
 * runSimulation runs in a separate thread.
 */

class Physics{
    private:
        std::unique_ptr<btDefaultCollisionConfiguration> m_collision_configuration;
        std::unique_ptr<btCollisionDispatcher> m_dispatcher;
        std::unique_ptr<btBroadphaseInterface> m_overlapping_pair_cache;
        std::unique_ptr<btSequentialImpulseConstraintSolver> m_solver;
        std::unique_ptr<btDiscreteDynamicsWorld> m_dynamics_world;

        /*
         * Starts Bullet.
         */
        void init(const btVector3& gravity);

        /*
         * Runs the simulation, is launched as a separate thead through startSimulation, uses the
         * thread monitor to synchronize itself with the main thead.
         */
        void runSimulation(int max_sub_steps);
        void noticeLogic();
        void waitLogic();

        /*
         * Applies gravity, should be a n-body simulation in the future when we have more planets.
         */
        void applyGravity();

        BaseApp* m_app;

        double m_delta_t, m_secs_since_j2000; // m_delta_t in s
        std::thread m_thread_simulation;
        bool m_simulation_paused, m_end_simulation;
        struct thread_monitor* m_thread_monitor;
    public:
        Physics();

        /*
         * Constructor
         *
         * @gravity: gravity direction, should be btVector(0.0, 0.0, 0.0) since we apply our own 
         * gravity.
         * @thread_monitor: thread monitor object used to synchronize ourselves with the main app.
         * @m_app: pointer of the app
         */
        Physics(const btVector3& gravity, thread_monitor* thread_monitor, BaseApp* app);
        ~Physics();

        /*
         * Adds a rigid body to the dynamics world. This method should not be called when the 
         * physics thread is running. Instead, use the command buffers of AssetManager or
         * AssetManagerInterface. To understand collision groups, collision masks and collision
         * flags look here:
         * https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=11865
         *
         * @body: pointer to the rigid body, this class does not take ownership of the rigid 
         * bodies.
         * @group: collision group of the body, we use custom collision groups (defined at the
         * top of this file).
         * @mask: collision mask, essentialy what should collide with this body.
         */
        void addRigidBody(btRigidBody* body, short group, short mask);

        /*
         * Adds a constraint to the dynamics world. This method should not be called when the 
         * physics thread is running, use command buffers instead.
         *
         * @constraint: pointer to the constraint
         * @disable_collision_between_bodies: if true disables the collision between the two bodies
         */
        void addConstraint(btTypedConstraint *constraint, bool disable_collision_between_bodies);

        /*
         * Removes a constraint, this method should not be called when the physics thread is 
         * running, use command buffers instead.
         *
         * @constraint: pointer to the constraint we want to remove.
         */
        void removeConstraint(btTypedConstraint *constraint);

        /*
         * Removes a rigid body from the dynamics world, this method should not be called when the
         * physics thread is running, use command buffers instead.
         *
         * @body: pointer to the body we want to remove.
         */
        void removeBody(btRigidBody* body);

        /*
         * Tests a ray, if it collides with an object it returns the pointer to that object. Mainly
         * used in the editor because it uses single precision. The collision group of this ray is
         * set to 1 (CG_DEFAULT) by default, so it will collide with everything. This method is 
         * thread safe (data races might still occur).
         *
         * @ray_start_world: start of the ray, single precision.
         * @ray_end_world: end of the ray, single precision.
         */
        Object* testRay(const math::vec3& ray_start_world, const math::vec3& ray_end_world) const;

        /*
         * The same as testRay, but you can provide the ray_callback with a custom collision group.
         * It uses double precision.
         *
         * @ray_callback: callback given to the rayTest method of the dynamics world.
         * @ray_start: start of the ray.
         * @ray_end: end of the ray
         */
        Object* testRay(btCollisionWorld::ClosestRayResultCallback& ray_callback, const btVector3& ray_start, const btVector3& ray_end) const;

        /*
         * Updates the AABB of the rigid body, may not be thread safe.
         *
         * @body: pointer to the rigid body.
         */
        void updateCollisionWorldSingleAABB(btRigidBody* body);

        /*
         * Returns the average load time of the physics thread in microseconds.
         */
        double getAverageLoadTime() const;

        /*
         * Methods to get a pointer to the dynamics world object.
         */
        const btDiscreteDynamicsWorld* getDynamicsWorld() const;
        btDiscreteDynamicsWorld* getDynamicsWorld();

        /*
         * Returns how much time has passed since the reference epoch, on the solar system that
         * should be J2000. The units is seconds.
         */
        double getCurrentTime() const;

        /*
         * Starts the simulation by launching a thread with the method runSimulation. To make the
         * simulation step use the thread monitor.
         *
         * @time_step: time step of each tick.
         * @max_sub_steps: maximum number of internal substeps of Bullet. Ideally it should be more
         * than 1, since we have a fixed time step. However, multiple substeps interferes with the
         * creation of the render buffers. For some reason, the coordinates of the objects appear
         * shifted from the camera because of some interpolation that Bullet does. There may be a
         * method to fix this, but I have found nothing so far. I found someone with the same 
         * problem a while ago, but I can't find the page now.
         */
        void startSimulation(int max_sub_steps);

        /*
         * Wakes up the physics thread and tells it to stop, then the thread joins.
         */
        void stopSimulation();

        /*
         * Pauses or resumes the simulation. The thread will still step but Bullet won't update and
         * gravity won't be applied.
         *
         * @stop_simulation: if true, the simulation is paused.
         */
        void pauseSimulation(bool stop_simulation);
};


#endif
