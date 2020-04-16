#include "BtWrapper.hpp"
#include "Object.hpp"


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

BtWrapper::BtWrapper(const btVector3& gravity, buffer* buffer1, buffer* buffer2, std::mutex* buff1_lock,
                     std::mutex* buff2_lock, buffer_manager* manager){
    m_buffer1 = buffer1;
    m_buffer2 = buffer2;
    m_buffer1_lock = buff1_lock;
    m_buffer2_lock = buff2_lock;
    m_last_updated = manager;

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
    std::chrono::system_clock::time_point loop_start = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point loop_start_load = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point previous_loop_start = std::chrono::system_clock::now();
    double accumulated_load_time = 0.0, accumulated_sleep_time = 0.0, max_delta = (1./60.)*1000.;
    int ticks_since_last_update = 0;

    while(!m_end_simulation){
        loop_start = std::chrono::system_clock::now();
        std::chrono::duration<double, std::milli> load_time = loop_start - loop_start_load;

        if(load_time.count() < max_delta){
            std::chrono::duration<double, std::milli> delta_ms(max_delta - load_time.count());
            auto delta_ms_duration = std::chrono::duration_cast<std::chrono::milliseconds>(delta_ms);
            std::this_thread::sleep_for(std::chrono::milliseconds(delta_ms_duration.count()));
        }

        loop_start_load = std::chrono::system_clock::now();

        if(!m_simulation_paused){
            m_dynamicsWorld->stepSimulation(time_step , max_sub_steps);
            updateBuffers();
        }

        m_elapsed_time += loop_start - previous_loop_start;

        ticks_since_last_update++;
        accumulated_load_time += load_time.count();
        accumulated_sleep_time += max_delta - load_time.count();

        if(ticks_since_last_update == 60){
            ticks_since_last_update = 0;
            m_average_load = accumulated_load_time / 60.0;
            m_average_sleep = accumulated_sleep_time / 60.0;
            accumulated_load_time = 0.0;
            accumulated_sleep_time = 0.0;

            std::cout << std::setfill('0') << std::setw(2) << (int(m_elapsed_time.count()) / 60000*60) % 60 
                      << ":" << std::setfill('0') << std::setw(2) << (int(m_elapsed_time.count()) / 60000) % 60
                      << ":" << std::setfill('0') << std::setw(2) << (int(m_elapsed_time.count()) / 1000) % 60 << std::endl;
        }
        previous_loop_start = loop_start;
    }
}


double BtWrapper::getAverageLoadTime() const{
    return m_average_load;
}


double BtWrapper::getAverageSleepTime() const{
    return m_average_sleep;
}


void BtWrapper::updateBuffer(buffer* buffer_){
    const btCollisionObjectArray& col_object_array = m_dynamicsWorld->getCollisionObjectArray();

    buffer_->clear();
    for(int i=0; i<col_object_array.size(); i++){
        Object* obj = static_cast<Object *>(col_object_array.at(i)->getUserPointer());
        buffer_->emplace_back(object_transform{obj, obj->getRigidBodyTransformSingle()});
    }
}


void BtWrapper::updateBuffers(){
    /*std::chrono::duration<double, std::micro> time;
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    std::chrono::system_clock::time_point end;*/
    

    if(*m_last_updated == buffer_2 || *m_last_updated == none){
        if(m_buffer1_lock->try_lock()){
            updateBuffer(m_buffer1);
            *m_last_updated = buffer_1;
            m_buffer1_lock->unlock();
        }
        else{
            m_buffer2_lock->lock(); // very unlikely to not get the lock
            updateBuffer(m_buffer2);
            *m_last_updated = buffer_2;
            m_buffer2_lock->unlock();
        }
    }
    else{
        if(m_buffer2_lock->try_lock()){
            updateBuffer(m_buffer2);
            *m_last_updated = buffer_2;
            m_buffer2_lock->unlock();
        }
        else{
            m_buffer1_lock->lock();
            updateBuffer(m_buffer1);
            *m_last_updated = buffer_1;
            m_buffer1_lock->unlock();
        }
    }
    /*end = std::chrono::system_clock::now();
    time = end - start;
    std::cout << "copy time: " << time.count() << std::endl;*/
}
