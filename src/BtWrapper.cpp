#include "BtWrapper.hpp"


BtWrapper::BtWrapper(){
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_overlappingPairCache = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver;
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);
    m_simulation_paused = true;
    m_end_simulation = false;
    m_average_load = 0.0;
    m_average_sleep = 0.0;

    log("BtWrapper: starting dynamics world");
    std::cout << "BtWrapper: starting dynamics world" << std::endl;

    m_dynamicsWorld->setGravity(btVector3(0.0, -9.81, 0.0));
}


BtWrapper::BtWrapper(const btVector3& gravity){
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_overlappingPairCache = new btDbvtBroadphase();
    m_solver = new btSequentialImpulseConstraintSolver;
    m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);
    m_simulation_paused = true;
    m_end_simulation = false;
    m_average_load = 0.0;
    m_average_sleep = 0.0;
    
    log("BtWrapper: starting dynamics world");
    std::cout << "BtWrapper: starting dynamics world" << std::endl;
    
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


void BtWrapper::startSimulation(btScalar time_step, int max_sub_steps){
    m_thread_simulation = std::thread(&BtWrapper::runSimulation, this, time_step, max_sub_steps);
    log("BtWrapper: starting simulation, thread launched");
}


void BtWrapper::stopSimulation(){
    m_end_simulation = true;
    m_thread_simulation.join();
    log("BtWrapper: simulation stopped, thread joined");
}


void BtWrapper::pauseSimulation(bool stop_simulation){
    m_simulation_paused = stop_simulation;
}


void BtWrapper::runSimulation(btScalar time_step, int max_sub_steps){
    std::chrono::system_clock::time_point start_current = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point start_last = std::chrono::system_clock::now();
    double max_delta = (1./60.)*1000.;
    double accumulated_load_time = 0.0, accumulated_sleep_time = 0.0;
    int ticks_since_last_update = 0;

    while(!m_end_simulation){
        start_current = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> load_time = start_current - start_last;

        if(load_time.count() < max_delta){
            std::chrono::duration<double, std::milli> delta_ms(max_delta - load_time.count());
            auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
        }

        start_last = std::chrono::system_clock::now();

        if(!m_simulation_paused) // deal with this better
            m_dynamicsWorld->stepSimulation(time_step, max_sub_steps);

        std::chrono::duration<double, std::milli> sleep_time = start_last - start_current;

        ticks_since_last_update += 1;
        accumulated_load_time += load_time.count();
        accumulated_sleep_time += sleep_time.count();

        if(ticks_since_last_update == 60){
            ticks_since_last_update = 0;
            m_average_load = accumulated_load_time / 60.0;
            m_average_sleep = accumulated_sleep_time / 60.0;
            accumulated_load_time = 0.0;
            accumulated_sleep_time = 0.0;
            //std::cout << "Work time: " << std::setprecision(3) << m_average_load << "ms - sleep time: " << std::setprecision(3) << m_average_sleep << "ms" << std::endl;
        }
    }
}


double BtWrapper::getAverageLoadTime() const{
    return m_average_load;
}


double BtWrapper::getAverageSleepTime() const{
    return m_average_sleep;
}

