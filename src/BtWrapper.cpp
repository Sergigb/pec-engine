#include "BtWrapper.hpp"


BtWrapper::BtWrapper(){
    init(btVector3(0.0, -9.81, 0.0));
}


BtWrapper::BtWrapper(const btVector3& gravity){
    init(gravity);
}


void BtWrapper::init(const btVector3& gravity){
    m_collision_configuration.reset(new btDefaultCollisionConfiguration());
    m_dispatcher.reset(new btCollisionDispatcher(m_collision_configuration.get()));
    m_overlapping_pair_cache.reset(new btDbvtBroadphase());
    m_solver.reset(new btSequentialImpulseConstraintSolver);
    m_dynamics_world.reset(new btDiscreteDynamicsWorld(m_dispatcher.get(), m_overlapping_pair_cache.get(), m_solver.get(), m_collision_configuration.get()));

    log("Starting dynamics world");
    std::cout << "Starting dynamics world" << std::endl;
    
    m_dynamics_world->setGravity(gravity);
}


BtWrapper::~BtWrapper(){
    // nothing ?
}


void BtWrapper::addRigidBody(btRigidBody* body){
    m_dynamics_world->addRigidBody(body);
}


void BtWrapper::stepSimulation(btScalar time_step, int max_sub_steps){
    m_dynamics_world->stepSimulation(time_step, max_sub_steps);
}


void BtWrapper::deleteBody(btRigidBody* body){
    // this leaks vvvv, not sure why
    m_dynamics_world->removeRigidBody(body);  // the instance of the object still has to be deleted
}


Object* BtWrapper::testRay(const math::vec3& ray_start_world, const math::vec3& ray_end_world) const{
    btCollisionWorld::ClosestRayResultCallback ray_callback(
            btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
    m_dynamics_world->rayTest(
            btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]), 
            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]),
            ray_callback);

    if(ray_callback.hasHit()) {
        Object* obj = static_cast<Object *>(ray_callback.m_collisionObject->getUserPointer());
        return obj;
    }else{
        return nullptr;
    }
}


void BtWrapper::addConstraint(btTypedConstraint *constraint, bool disable_collision_between_bodies){
    m_dynamics_world->addConstraint(constraint, disable_collision_between_bodies);
}


void BtWrapper::updateCollisionWorldSingleAABB(btRigidBody* body){
    m_dynamics_world->getCollisionWorld()->updateSingleAabb(body);
}

