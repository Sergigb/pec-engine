#include <algorithm>

#include "Object.hpp"
#include "BtWrapper.hpp"
#include "Model.hpp"
#include "id_manager.hpp"


Object::Object(){
}


Object::Object(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, std::uint32_t base_id){
    m_mesh_color = math::vec4(1.0, 1.0, 1.0, 1.0);
    m_bt_wrapper = bt_wrapper;
    m_model = model;
    m_has_transform = false;
    m_col_shape = col_shape;
    m_mass = mass;
    m_base_id = base_id;
    m_object_name = "unnamed";
    m_fancy_name = "unnamed";
    m_render_ignore = false;
    create_id(m_unique_id, PART_SET);
    m_alpha = 1.0;
    m_col_group = 1;
    m_col_filters = -1;
}


Object::Object(const Object& obj) : std::enable_shared_from_this<Object>(){
    m_model = obj.m_model;
    m_bt_wrapper = obj.m_bt_wrapper;
    m_mesh_color = obj.m_mesh_color;
    m_mesh_transform = obj.m_mesh_transform;
    m_has_transform = obj.m_has_transform;
    m_base_id = obj.m_base_id;
    m_object_name = obj.m_object_name;
    m_fancy_name = obj.m_fancy_name;
    m_col_shape = obj.m_col_shape;
    m_mass = obj.m_mass;
    m_render_ignore = false;
    create_id(m_unique_id, PART_SET);
    m_alpha = 1.0;
    m_col_group = obj.m_col_group;
    m_col_filters = obj.m_col_filters;

    m_body.reset(nullptr);
}


Object::~Object(){
    if(m_body != nullptr){ // this should not be necessary, objects' rigid bodies should be removed before this constructor is called
        m_bt_wrapper->removeBody(m_body.get());
    }
    remove_id(m_unique_id, PART_SET);
}


void Object::addBody(const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation){
    btVector3 local_inertia_ = local_inertia;
    btTransform start_transform;

    start_transform.setIdentity();
    start_transform.setOrigin(origin);
    start_transform.setRotation(initial_rotation);

    bool is_dynamic = (m_mass != 0.f);

    if(is_dynamic)
        m_col_shape->calculateLocalInertia(m_mass, local_inertia_);

    m_motion_state.reset(new btDefaultMotionState(start_transform));
    btRigidBody::btRigidBodyConstructionInfo rb_info(m_mass, m_motion_state.get(), m_col_shape, local_inertia_);
    m_body.reset(new btRigidBody(rb_info));

    m_bt_wrapper->addRigidBody(m_body.get(), m_col_group, m_col_filters);
    m_body->setUserPointer((void*)this);
    m_body->setActivationState(DISABLE_DEACTIVATION);
}


void Object::removeBody(){
    if(m_body != nullptr){
        m_bt_wrapper->removeBody(m_body.get());
    }
}


int Object::render(){
    math::mat4 body_transform = getRigidBodyTransformSingle();
 
    if(m_has_transform){
        body_transform = body_transform * m_mesh_transform;
    }

    m_model->setMeshColor(math::vec4(m_mesh_color, m_alpha));
    return m_model->render(body_transform);
}


int Object::render(math::mat4 body_transform){
    if(m_has_transform){
        body_transform = body_transform * m_mesh_transform;
    }

    m_model->setMeshColor(math::vec4(m_mesh_color, m_alpha));
    return m_model->render(body_transform);
}


void Object::setColor(math::vec3 color){
    m_mesh_color = color;
}


void Object::setMeshScale(const math::vec3& scale){
    m_mesh_transform = math::identity_mat4();
    m_has_transform = true;
    m_mesh_transform.m[0] = scale.v[0];
    m_mesh_transform.m[5] = scale.v[1];
    m_mesh_transform.m[10] = scale.v[2];
}


void Object::setMeshScale(float scale){
    m_mesh_transform = math::identity_mat4();
    m_has_transform = true;
    m_mesh_transform.m[0] = scale;
    m_mesh_transform.m[5] = scale;
    m_mesh_transform.m[10] = scale;
}


void Object::setMeshTransform(const math::mat4& transform){
    m_mesh_transform = transform;
}


void Object::applyCentralForce(const btVector3& force){
    m_body->activate(true);
    m_body->applyCentralForce(force);
}


void Object::applyTorque(const btVector3& torque){
    m_body->activate(true);
    m_body->applyTorque(torque);
}


void Object::setMotionState(const btVector3& origin, const btQuaternion& initial_rotation){
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(origin);
    transform.setRotation(initial_rotation);

    m_motion_state.reset(new btDefaultMotionState(transform));

    m_body->setMotionState(m_motion_state.get());
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


void Object::getRigidBodyTransformSingle(math::mat4& body_transform) const{
    btTransform trans;
    double body_transform_double[16];

    m_body->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(body_transform_double);
    std::copy(body_transform_double, body_transform_double + 16, body_transform.m);
}


void Object::getRigidBodyTransformDouble(double* mat4) const{
    btTransform trans;

    m_body->getMotionState()->getWorldTransform(trans);
    trans.getOpenGLMatrix(mat4);
}


void Object::setName(std::string name){
    m_object_name = name;
}


void Object::setFancyName(std::string name){
    m_fancy_name = name;
}


void Object::getName(std::string& name) const{
    name = m_object_name;
}


void Object::getFancyName(std::string& name) const{
    name = m_fancy_name;
}


std::uint32_t Object::getUniqueId() const{
    return m_unique_id;
}


std::uint32_t Object::getBaseId() const{
    return m_base_id;
}


std::shared_ptr<Object> Object::getSharedPtr(){
    return shared_from_this();
}


bool Object::renderIgnore() const{
    return m_render_ignore;
}


void Object::setRenderIgnore(){
    m_render_ignore = true;
}


void Object::setAlpha(float alpha){
    m_alpha = alpha;
}


void Object::setCollisionGroup(short cg_group){
    m_col_group = cg_group;
}


void Object::setCollisionFilters(short cg_filters){
    m_col_filters = cg_filters;
}


short Object::getCollisionGroup() const{
    return m_col_group;
}


short Object::getCollisionFilters() const{
    return m_col_filters;
}


void Object::renderOther(){

}


void Object::onEditorRightMouseButton(){

}


