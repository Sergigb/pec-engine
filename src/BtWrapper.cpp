#include "BtWrapper.hpp"
#include "Object.hpp"


BtWrapper::BtWrapper(){
    init(btVector3(0.0, -9.81, 0.0));
}

BtWrapper::BtWrapper(const btVector3& gravity, render_buffers* buff_manager, thread_monitor* thread_monitor){
    m_thread_monitor = thread_monitor;
    m_buffers = buff_manager;

    init(gravity);
}


void BtWrapper::init(const btVector3& gravity){
    m_collision_configuration.reset(new btDefaultCollisionConfiguration());
    m_dispatcher.reset(new btCollisionDispatcher(m_collision_configuration.get()));
    m_overlapping_pair_cache.reset(new btDbvtBroadphase());
    m_solver.reset(new btSequentialImpulseConstraintSolver);
    m_dynamics_world.reset(new btDiscreteDynamicsWorld(m_dispatcher.get(), m_overlapping_pair_cache.get(), m_solver.get(), m_collision_configuration.get()));
    m_simulation_paused = true;
    m_end_simulation = false;
    m_average_load = 0.0;
    
    log("BtWrapper: starting dynamics world");
    std::cout << "BtWrapper: starting dynamics world" << std::endl;
    
    m_dynamics_world->setGravity(gravity);
}


BtWrapper::~BtWrapper(){
    // nothing ?
}


void BtWrapper::addRigidBody(btRigidBody* body){
    m_dynamics_world->addRigidBody(body);
}


void BtWrapper::removeBody(btRigidBody* body){
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
        Object* obj = static_cast<Object*>(ray_callback.m_collisionObject->getUserPointer());
        return obj;
    }else{
        return nullptr;
    }
}


void BtWrapper::addConstraint(btTypedConstraint *constraint, bool disable_collision_between_bodies){
    m_dynamics_world->addConstraint(constraint, disable_collision_between_bodies);
}


void BtWrapper::removeConstraint(btTypedConstraint *constraint){
    m_dynamics_world->removeConstraint(constraint);
}


void BtWrapper::updateCollisionWorldSingleAABB(btRigidBody* body){
    m_dynamics_world->getCollisionWorld()->updateSingleAabb(body);
}


void BtWrapper::startSimulation(btScalar time_step, int max_sub_steps){
    m_thread_simulation = std::thread(&BtWrapper::runSimulation, this, time_step, max_sub_steps);
    log("BtWrapper: starting simulation, thread launched");
}


void BtWrapper::stopSimulation(){
    m_end_simulation = true;
    {
        std::unique_lock<std::mutex> lck2(m_thread_monitor->mtx_start);
        m_thread_monitor->worker_start = true;
        m_thread_monitor->cv_start.notify_all();
    }
    m_thread_simulation.join();
    log("BtWrapper: simulation stopped, thread joined");
}


void BtWrapper::pauseSimulation(bool stop_simulation){
    m_simulation_paused = stop_simulation;
}


void BtWrapper::noticeLogic(){
    // logic thread notice
    std::unique_lock<std::mutex> lck2(m_thread_monitor->mtx_end);
    m_thread_monitor->worker_ended = true;
    m_thread_monitor->cv_end.notify_all();
}


void BtWrapper::waitLogic(){
    // logic thread wait
    std::unique_lock<std::mutex> lck(m_thread_monitor->mtx_start);
    while(!m_thread_monitor->worker_start){
        m_thread_monitor->cv_start.wait(lck);
    }
    m_thread_monitor->worker_start = false;
}


void BtWrapper::runSimulation(btScalar time_step, int max_sub_steps){
    std::chrono::steady_clock::time_point loop_start_load, loop_end_load;
    double accumulated_load_time = 0.0;
    int ticks_since_last_update = 0;

    waitLogic();
    while(!m_end_simulation){
        loop_start_load = std::chrono::steady_clock::now();

        if(!m_simulation_paused){
            m_dynamics_world->stepSimulation(time_step , max_sub_steps);
        }
        updateBuffers();

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


double BtWrapper::getAverageLoadTime() const{
    return m_average_load;
}


void BtWrapper::updateBuffer(std::vector<object_transform>* buffer_){
    const btCollisionObjectArray& col_object_array = m_dynamics_world->getCollisionObjectArray();

    buffer_->clear();
    for(int i=0; i<col_object_array.size(); i++){
        Object* obj = static_cast<Object *>(col_object_array.at(i)->getUserPointer());
        if(obj->renderIgnore()){ // ignore if the object should be destroyed
            continue;
        }
        try{
            std::shared_ptr<Object> obj_sptr = obj->getSharedPtr();    
            buffer_->emplace_back(object_transform{obj_sptr, obj->getRigidBodyTransformSingle()});
        }
        catch(std::bad_weak_ptr& e) {
            std::string name;
            obj->getFancyName(name);
            std::cout << "BtWrapper::updateBuffer - Warning, weak ptr for object " << name << " with id " << obj->getBaseId() << '\n';
        }
    }
}


void BtWrapper::updateBuffers(){
    /*std::chrono::duration<double, std::micro> time;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end;*/
    

    if(m_buffers->last_updated == buffer_2 || m_buffers->last_updated == none){
        if(m_buffers->buffer1_lock.try_lock()){
            updateBuffer(&m_buffers->buffer1);
            m_buffers->last_updated = buffer_1;
            m_buffers->buffer1_lock.unlock();
        }
        else{
            m_buffers->buffer2_lock.lock(); // very unlikely to not get the lock
            updateBuffer(&m_buffers->buffer2);
            m_buffers->last_updated = buffer_2;
            m_buffers->buffer2_lock.unlock();
        }
    }
    else{
        if(m_buffers->buffer2_lock.try_lock()){
            updateBuffer(&m_buffers->buffer2);
            m_buffers->last_updated = buffer_2;
            m_buffers->buffer2_lock.unlock();
        }
        else{
            m_buffers->buffer1_lock.lock();
            updateBuffer(&m_buffers->buffer1);
            m_buffers->last_updated = buffer_1;
            m_buffers->buffer1_lock.unlock();
        }
    }
    /*end = std::chrono::steady_clock::now();
    time = end - start;
    std::cout << "copy time: " << time.count() << std::endl;*/
}
