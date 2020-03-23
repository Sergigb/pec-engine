#include "BtWrapper.hpp"


BtWrapper::BtWrapper(){
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_overlappingPairCache = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver;
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);

    m_dynamicsWorld->setGravity(btVector3(0.0, -9.81, 0.0));
}


BtWrapper::BtWrapper(const btVector3& gravity, const WindowHandler* window_handler, const Input* input){
    m_window_handler = window_handler;
    m_input = input;

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

