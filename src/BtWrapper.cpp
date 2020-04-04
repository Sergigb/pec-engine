#include "BtWrapper.hpp"


BtWrapper::BtWrapper(){
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_overlappingPairCache = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver;
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);

    m_dynamicsWorld->setGravity(btVector3(0.0, -9.81, 0.0));
}


BtWrapper::BtWrapper(const btVector3& gravity){
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_overlappingPairCache = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver;
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);

    log("Starting dynamics world");
    std::cout << "Starting dynamics world" << std::endl;
    
    m_dynamicsWorld->setGravity(gravity);
}


BtWrapper::~BtWrapper(){
    // the deletion of the body from the dynamics world and the deletion of the rigid bodies and their motion states is handled in the object class
    
    delete m_dynamicsWorld;
    delete m_solver;
    delete m_overlappingPairCache;
    delete m_dispatcher;
    delete m_collisionConfiguration;
}


void BtWrapper::addRigidBody(btRigidBody* body){
    m_dynamicsWorld->addRigidBody(body);
}


void BtWrapper::stepSimulation(btScalar time_step, int max_sub_steps){
    m_dynamicsWorld->stepSimulation(time_step, max_sub_steps);
}


void BtWrapper::deleteBody(btRigidBody* body){
    // this leaks vvvv, not sure why
    m_dynamicsWorld->removeRigidBody(body);  // the instance of the object still has to be deleted
}


Object* BtWrapper::testRay(const math::vec3& ray_start_world, const math::vec3& ray_end_world) const{
    btCollisionWorld::ClosestRayResultCallback ray_callback(
            btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
    m_dynamicsWorld->rayTest(
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
    m_dynamicsWorld->addConstraint(constraint, disable_collision_between_bodies);
}


void BtWrapper::updateCollisionWorldSingleAABB(btRigidBody* body){
    m_dynamicsWorld->getCollisionWorld()->updateSingleAabb(body);
}

