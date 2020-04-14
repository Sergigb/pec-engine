#include "Object.hpp"


Object::Object(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation, btScalar mass){
    //btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
    btVector3 local_inertia_ = local_inertia;

    m_mesh_color = math::vec3(1.0, 1.0, 1.0);
    m_bt_wrapper = bt_wrapper;
    m_model = model;
    m_scale = 1.0;

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
    m_body->setUserPointer((void*)this);
}

Object::~Object(){
    m_bt_wrapper->deleteBody(m_body);
    delete m_body->getMotionState();
    delete m_body;
}


int Object::render(){
    math::mat4 body_transform = getRigidBodyTransformSingle();
 
    if(m_scale != 1.0){
        body_transform = body_transform * m_scale_transform;
    }

    m_model->setMeshColor(m_mesh_color);
    return m_model->render(body_transform);
}


int Object::render(math::mat4 body_transform){
    m_model->setMeshColor(m_mesh_color);
    return m_model->render(body_transform);
}


void Object::setColor(math::vec3 color){
    m_mesh_color = color;
}


void Object::setMeshScale(float scale){
    m_scale_transform = math::identity_mat4();
    m_scale = scale;
    m_scale_transform.m[0] = scale;
    m_scale_transform.m[5] = scale;
    m_scale_transform.m[10] = scale;
}


void Object::applyCentralForce(const btVector3& force){
    m_body->activate(true);
    m_body->applyCentralForce(force);
}


void Object::applyTorque(const btVector3& torque){
    m_body->activate(true);
    m_body->applyTorque(torque);
}


btRigidBody* Object::getRigidBody(){
    return m_body;
}


void Object::setMotionState(const btVector3& origin, const btQuaternion& initial_rotation){
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(origin);
    transform.setRotation(initial_rotation);

    btDefaultMotionState* motion_state = new btDefaultMotionState(transform);

    m_body->setMotionState(motion_state);
}


void Object::activate(bool activate){
    m_body->activate(activate);
}


math::mat4 Object::getRigidBodyTransformSingle() const{
    btTransform trans;
    math::mat4 body_transform;
    double body_transform_double[16];

    m_body->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(body_transform_double);
    std::copy(body_transform_double, body_transform_double + 16, body_transform.m);

    return body_transform;
}


void Object::getRigidBodyTransformDouble(double* mat4) const{
    btTransform trans;

    m_body->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(mat4);
}
