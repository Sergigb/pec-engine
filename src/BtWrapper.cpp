#include "BtWrapper.hpp"


BtWrapper::BtWrapper(const btVector3& gravity){
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_overlappingPairCache = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver;
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);

    m_dynamicsWorld->setGravity(gravity);
}


BtWrapper::~BtWrapper(){
    for(int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--){
        btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        if(body && body->getMotionState()){
            delete body->getMotionState();
        }
        m_dynamicsWorld->removeCollisionObject(obj);
        delete obj;
    }

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
    m_dynamicsWorld->removeRigidBody(body);  // the instance of the object still has to be deleted
}

