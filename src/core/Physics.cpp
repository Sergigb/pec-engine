#include <mutex>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include "Physics.hpp"
#include "log.hpp"
#include "multithreading.hpp"
#include "../assets/Object.hpp"


Physics::Physics(){
    init(btVector3(0.0, -9.81, 0.0));
}

Physics::Physics(const btVector3& gravity, thread_monitor* thread_monitor){
    m_thread_monitor = thread_monitor;

    init(gravity);
}


void Physics::init(const btVector3& gravity){
    m_collision_configuration.reset(new btDefaultCollisionConfiguration());
    m_dispatcher.reset(new btCollisionDispatcher(m_collision_configuration.get()));
    m_overlapping_pair_cache.reset(new btDbvtBroadphase());
    m_solver.reset(new btSequentialImpulseConstraintSolver);
    m_dynamics_world.reset(new btDiscreteDynamicsWorld(m_dispatcher.get(), m_overlapping_pair_cache.get(), m_solver.get(), m_collision_configuration.get()));
    btGImpactCollisionAlgorithm::registerAlgorithm(m_dispatcher.get());

    m_simulation_paused = true;
    m_end_simulation = false;
    m_average_load = 0.0;
    
    log("Physics: starting dynamics world");
    std::cout << "Physics: starting dynamics world" << std::endl;

    //btOverlapFilterCallback*filtercbk=new myFilterCallback();
    //m_dynamics_world->getPairCache()->setOverlapFilterCallback(filtercbk);
    
    m_dynamics_world->setGravity(gravity);
}


Physics::~Physics(){
    m_dynamics_world.reset(nullptr);
    m_solver.reset(nullptr);
    m_overlapping_pair_cache.reset(nullptr);
    m_dispatcher.reset(nullptr);
    m_collision_configuration.reset(nullptr);
}


void Physics::addRigidBody(btRigidBody* body, short group, short mask){
    m_dynamics_world->addRigidBody(body, group, mask);
}


void Physics::removeBody(btRigidBody* body){
    // this leaks vvvv, not sure why
    m_dynamics_world->removeRigidBody(body);  // the instance of the object still has to be deleted
}


Object* Physics::testRay(const math::vec3& ray_start_world, const math::vec3& ray_end_world) const{
    btCollisionWorld::ClosestRayResultCallback ray_callback(
            btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
    m_dynamics_world->rayTest(
            btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]), 
            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]),
            ray_callback);

    if(ray_callback.hasHit()) {
        Object* obj = static_cast<Object*>(ray_callback.m_collisionObject->getUserPointer());
        return obj;
    }else{
        return nullptr;
    }
}


Object* Physics::testRay(btCollisionWorld::ClosestRayResultCallback& ray_callback, const btVector3& ray_start, const btVector3& ray_end) const{
    m_dynamics_world->rayTest(ray_start, ray_end, ray_callback);

    if(ray_callback.hasHit()) {
        Object* obj = static_cast<Object*>(ray_callback.m_collisionObject->getUserPointer());
        return obj;
    }else{
        return nullptr;
    }
}


void Physics::addConstraint(btTypedConstraint *constraint, bool disable_collision_between_bodies){
    m_dynamics_world->addConstraint(constraint, disable_collision_between_bodies);
}


void Physics::removeConstraint(btTypedConstraint *constraint){
    m_dynamics_world->removeConstraint(constraint);
}


void Physics::updateCollisionWorldSingleAABB(btRigidBody* body){
    m_dynamics_world->getCollisionWorld()->updateSingleAabb(body);
}


void Physics::startSimulation(btScalar time_step, int max_sub_steps){
    m_thread_simulation = std::thread(&Physics::runSimulation, this, time_step, max_sub_steps);
    log("Physics: starting simulation, thread launched");
}


void Physics::stopSimulation(){
    m_end_simulation = true;
    {
        std::unique_lock<std::mutex> lck2(m_thread_monitor->mtx_start);
        m_thread_monitor->worker_start = true;
        m_thread_monitor->cv_start.notify_all();
    }
    m_thread_simulation.join();
    log("Physics: simulation stopped, thread joined");
}


void Physics::pauseSimulation(bool stop_simulation){
    m_simulation_paused = stop_simulation;
}


void Physics::noticeLogic(){
    // logic thread notice
    std::unique_lock<std::mutex> lck2(m_thread_monitor->mtx_end);
    m_thread_monitor->worker_ended = true;
    m_thread_monitor->cv_end.notify_all();
}


void Physics::waitLogic(){
    // logic thread wait
    std::unique_lock<std::mutex> lck(m_thread_monitor->mtx_start);
    while(!m_thread_monitor->worker_start){
        m_thread_monitor->cv_start.wait(lck);
    }
    m_thread_monitor->worker_start = false;
}


void Physics::runSimulation(btScalar time_step, int max_sub_steps){
    std::chrono::steady_clock::time_point loop_start_load, loop_end_load;
    double accumulated_load_time = 0.0;
    int ticks_since_last_update = 0;

    waitLogic();
    while(!m_end_simulation){
        loop_start_load = std::chrono::steady_clock::now();

        if(!m_simulation_paused){
            applyGravity();
            m_dynamics_world->stepSimulation(time_step , max_sub_steps);
        }

        loop_end_load = std::chrono::steady_clock::now();

        std::chrono::duration<double, std::micro> load_time = loop_end_load - loop_start_load;
        accumulated_load_time += load_time.count();
        ticks_since_last_update++;

        if(ticks_since_last_update == 60){
            ticks_since_last_update = 0;
            m_average_load = accumulated_load_time / 60000.0;
            accumulated_load_time = 0.0;
        }

        noticeLogic();
        waitLogic();
    }
}


double Physics::getAverageLoadTime() const{
    return m_average_load;
}


const btDiscreteDynamicsWorld* Physics::getDynamicsWorld() const{
    return m_dynamics_world.get();
}


btDiscreteDynamicsWorld* Physics::getDynamicsWorld(){
    return m_dynamics_world.get();
}


// assuming that we only have 1 planet, and that's earth, this calculates the force applied by its gravity field
// this should be easy to optimize in the future, calculate gravity with the com of the vessel, and then apply f
// to individual objects 
#define EARTH_MASS 5973600000000000000000000.0
void Physics::applyGravity(){
    double acceleration;
    btCollisionObjectArray& col_object_array = m_dynamics_world->getCollisionObjectArray();

    // in the future this class should be able to acces the memory structures where the objects are saved, in order to avoid casts
    for(int i=0; i < col_object_array.size(); i++){
        btRigidBody* obj = static_cast<btRigidBody*>(col_object_array.at(i));
        const btVector3& object_origin = obj->getWorldTransform().getOrigin();
        double Rh = object_origin.norm(); // assuming earth's center of gravity is at the origin

        acceleration = GRAVITATIONAL_CONSTANT * (EARTH_MASS / (Rh*Rh));

        if(!(obj->getCollisionFlags() & (btCollisionObject::CF_KINEMATIC_OBJECT | btCollisionObject::CF_STATIC_OBJECT))){
            // we should keep the mass in the object
            btVector3 f = ((1 / obj->getInvMass()) * (-object_origin).normalize() * acceleration);
            obj->applyCentralForce(f);            
        }
    }
}

