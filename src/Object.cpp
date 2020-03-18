#include "Object.hpp"


Object::Object(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation, btScalar mass){
    //btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
    btVector3 local_inertia_ = local_inertia;

    m_mesh_color = math::vec3(1.0, 1.0, 1.0);
    m_bt_wrapper = bt_wrapper;
    m_model = model;

    /// Create Dynamic Objects
    btTransform start_transform;
    start_transform.setIdentity();
    start_transform.setOrigin(origin);
    start_transform.setRotation(initial_rotation);

    //rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);

    if (isDynamic)
        col_shape->calculateLocalInertia(mass, local_inertia_);

    //using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
    btDefaultMotionState* motion_state = new btDefaultMotionState(start_transform);
    btRigidBody::btRigidBodyConstructionInfo rb_info(mass, motion_state, col_shape, local_inertia_);
    m_body = new btRigidBody(rb_info);

    m_bt_wrapper->addRigidBody(m_body);
}

Object::~Object(){
    m_bt_wrapper->deleteBody(m_body);
    delete m_body->getMotionState();
    delete m_body;
}


int Object::render(){
    btTransform trans;
    math::mat4 body_transform;
    double body_transform_double[16];

    // at some point we should try going far away from the origin and check how the precision loss affects the rendering
    m_body->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(body_transform_double);
    std::copy(body_transform_double, body_transform_double + 16, body_transform.m); // implicit cast, maybe some compilers will complain?
    m_model->setMeshColor(m_mesh_color);
    return m_model->render(body_transform);
}


void Object::setColor(math::vec3 color){
    m_mesh_color = color;
}

