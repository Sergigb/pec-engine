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


Object* BtWrapper::testMousePick(float fb_width, float fb_height, float mouse_x, float mouse_y, const math::mat4& proj_matrix, const math::mat4& view_matrix, double dist){
    math::vec4 ray_start, ray_start_world;
    math::vec4 ray_end, ray_end_world;
    math::mat4 M;
    math::vec3 ray_dir, ray_end_world_ext;

    // the extraction of the ray could be a method in the camera class at some point, if we need it

    ray_start = math::vec4((mouse_x/fb_width - 0.5) * 2.0,
                           (mouse_y/fb_height - 0.5) * 2.0,
                           -1.0, 1.0);
    ray_end = math::vec4((mouse_x/fb_width - 0.5) * 2.0,
                         (mouse_y/fb_height - 0.5) * 2.0,
                         0.0, 1.0);

    M = math::inverse(proj_matrix * view_matrix);

    ray_start_world = M * ray_start;
    ray_start_world = ray_start_world / ray_start_world.v[3];
    ray_end_world = M * ray_end;
    ray_end_world = ray_end_world / ray_end_world.v[3];

    ray_dir = math::normalise(ray_end_world - ray_start_world);

    ray_end_world_ext = math::vec3(ray_start_world) + ray_dir * dist; // ray end extended according to dist (in meters)


    btCollisionWorld::ClosestRayResultCallback ray_callback(
            btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
            btVector3(ray_end_world_ext.v[0], ray_end_world_ext.v[1], ray_end_world_ext.v[2]));
    m_dynamicsWorld->rayTest(
            btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]), 
            btVector3(ray_end_world_ext.v[0], ray_end_world_ext.v[1], ray_end_world_ext.v[2]),
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

