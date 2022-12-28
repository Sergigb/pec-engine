#include <mutex>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include "Physics.hpp"
#include "BaseApp.hpp"
#include "AssetManager.hpp"
#include "log.hpp"
#include "multithreading.hpp"
#include "timing.hpp"
#include "RenderContext.hpp"
#include "../GUI/DebugOverlay.hpp"
#include "../assets/Object.hpp"
#include "../assets/PlanetarySystem.hpp"
#include "../assets/Planet.hpp"
#include "../assets/Vessel.hpp"
#include "../assets/BasePart.hpp"


Physics::Physics(){
}

Physics::Physics(BaseApp* app){
    m_thread_monitor = app->getThreadMonitor();
    m_app = app;
    m_delta_t = REAL_TIME_S;
    m_secs_since_j2000 = 0.0;
}


void Physics::initDynamicsWorld(const btVector3& gravity){
    m_collision_configuration.reset(new btDefaultCollisionConfiguration());
    m_dispatcher.reset(new btCollisionDispatcher(m_collision_configuration.get()));
    m_overlapping_pair_cache.reset(new btDbvtBroadphase());
    m_solver.reset(new btSequentialImpulseConstraintSolver);
    m_dynamics_world.reset(new btDiscreteDynamicsWorld(m_dispatcher.get(), m_overlapping_pair_cache.get(), m_solver.get(), m_collision_configuration.get()));
    btGImpactCollisionAlgorithm::registerAlgorithm(m_dispatcher.get());

    m_simulation_paused = true;
    m_end_simulation = false;
    
    log("Physics::initDynamicsWorld: starting dynamics world");
    std::cout << "Physics::initDynamicsWorld: starting dynamics world" << std::endl;

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


void Physics::startSimulation(int max_sub_steps){
    // initialize orbital elements
    double cents_since_j2000 = m_secs_since_j2000 / SECONDS_IN_A_CENTURY;
    m_app->getAssetManager()->m_planetary_system->updateOrbitalElements(cents_since_j2000);

    m_thread_simulation = std::thread(&Physics::runSimulation, this, max_sub_steps);
    log("Physics::startSimulation: starting simulation, thread launched");
}


void Physics::stopSimulation(){
    m_end_simulation = true;
    {
        std::unique_lock<std::mutex> lck2(m_thread_monitor->mtx_start);
        m_thread_monitor->worker_start = true;
        m_thread_monitor->cv_start.notify_all();
    }
    m_thread_simulation.join();
    log("Physics::stopSimulation: simulation stopped, thread joined");
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


void Physics::runSimulation(int max_sub_steps){
    physics_timing timing;
    double cents_since_j2000;
    DebugOverlay* debug_overlay = m_app->getRenderContext()->getDebugOverlay();
    PlanetarySystem* planetary_system = m_app->getAssetManager()->m_planetary_system.get();

    waitLogic();
    while(!m_end_simulation){
        timing.register_tp(TP_PHYSICS_START);
        cents_since_j2000 = m_secs_since_j2000 / SECONDS_IN_A_CENTURY;

        if(!m_simulation_paused){
            /* updating the orbital elements could be done in a different thread while the thigs
               below are updating, a buffering system could be used 

               and the same for the kinematics :)
               */

            // WARNING! THE ORDER OF THESE UPDATES IS IMPORTANT, WE NEED TO REVISE THEM
            planetary_system->updateOrbitalElements(cents_since_j2000);
            timing.register_tp(TP_ORBIT_END);
            planetary_system->updateKinematics();
            timing.register_tp(TP_KINEM_END);
            applyGravity();
            timing.register_tp(TP_GRAV_END);
            m_dynamics_world->stepSimulation(m_delta_t, max_sub_steps);

            m_secs_since_j2000 += m_delta_t;
        }

        timing.register_tp(TP_PHYSICS_END);
        timing.update(m_simulation_paused);
        debug_overlay->setPhysicsTimes(timing);

        noticeLogic();
        waitLogic();
    }
}


double Physics::getAverageLoadTime() const{
    return 0.0;
}


const btDiscreteDynamicsWorld* Physics::getDynamicsWorld() const{
    return m_dynamics_world.get();
}


btDiscreteDynamicsWorld* Physics::getDynamicsWorld(){
    return m_dynamics_world.get();
}


void Physics::applyGravityStar(double star_mass, btRigidBody* rbody, 
                               const btVector3& rbody_origin){
    double Rh = rbody_origin.norm(); // star is in the center of the space
    double acceleration = GRAVITATIONAL_CONSTANT * (star_mass / (Rh*Rh));
    btVector3 f = (1 / rbody->getInvMass()) * (-rbody_origin).normalize() * acceleration;
    rbody->applyCentralForce(f);
}


/*
    First steps towards a n-body simulation, this has much work to do:
     - when we are not time-warping (which is always because it's not implented) bullet integrates the forces and uses, apparently, symplectic Euler. I have no idea
       if sympletic Euler is better than Runge-Kutta methods for classical mechanics, or if other sympletic methods are better than symplectic Euler, I don't know because
       all this stuff sounds like fucking nonsense when I read it, so I'll have to dig more into this. This question is interesting:

        https://scicomp.stackexchange.com/questions/29149/what-does-symplectic-mean-in-reference-to-numerical-integrators-and-does-scip

    - this method might be critical to optimize, so much work can be done here
    - one possible optimization can be to calculate the total force applied to the vessel by using the CoM + total vessel mass. Then we can propotionally apply the force
    using the part's mass.
    - the planet's origin is stored as dmath::vec3, should be btVector3 to avoid conversions.
    - the gravity of the star is applied separately, which is weird but ok for single starts.

    I've verified that when the vessel is close to the earth the acceleration is 1.2, which is not 1 but close enough, this is because the earth is moving while our
    object starts stationary. 1.2 is close enough though. The next steps will be to verify that the simulation works more or less, also I'll try to implement orbital
    predictions. The first thing is to show the vessel's name on the Planetarium view's GUI.
 */
void Physics::applyGravity(){
    AssetManager* asset_manager = m_app->getAssetManager();
    const planet_map& planets = asset_manager->m_planetary_system->getPlanets();
    planet_map::const_iterator it;
    VesselMap::iterator it2;
    double star_mass = asset_manager->m_planetary_system->getStar().mass;

    // objects...
    for(uint i=0; i < asset_manager->m_objects.size(); i++){
        btRigidBody* body = asset_manager->m_objects.at(i)->m_body.get();
        const btVector3& object_origin = body->getWorldTransform().getOrigin();

        applyGravityStar(star_mass, body, object_origin);

        for(it = planets.begin(); it != planets.end(); it++){
            const orbital_data& data =  it->second->getOrbitalData();
            btVector3 pos_planet(data.pos.v[0], data.pos.v[1], data.pos.v[2]); // stupid conversion from dmath::vec3 to btVector3, just store pos as a btVector3 god damn it...
            double Rh = pos_planet.distance(object_origin);

            double acceleration = GRAVITATIONAL_CONSTANT * (data.m / (Rh*Rh));
            btVector3 f = (1 / body->getInvMass()) * (pos_planet - object_origin).normalize()
                          * acceleration;
            body->applyCentralForce(f);
        }
    }

    for(it2 = asset_manager->m_active_vessels.begin(); it2 != asset_manager->m_active_vessels.end(); it2++){
        std::vector<BasePart*>& parts = it2->second->getParts();

        for(uint i=0; i < parts.size(); i++){
            btRigidBody* body = parts.at(i)->m_body.get();
            const btVector3& object_origin = body->getWorldTransform().getOrigin();

            applyGravityStar(star_mass, body, object_origin);

            for(it = planets.begin(); it != planets.end(); it++){
                const orbital_data& data =  it->second->getOrbitalData();
                btVector3 pos_planet(data.pos.v[0], data.pos.v[1], data.pos.v[2]);
                double Rh = pos_planet.distance(object_origin);

                double acceleration = GRAVITATIONAL_CONSTANT * (data.m / (Rh*Rh));
                btVector3 f = (1 / body->getInvMass()) * (pos_planet - object_origin).normalize()
                               * acceleration;
                body->applyCentralForce(f);

            }
        }
    }
}


double Physics::getCurrentTime() const{
    return m_secs_since_j2000;
}


void Physics::setCurrentTime(double time){
    m_secs_since_j2000 = time;
}


