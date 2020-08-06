#include "BasePart.hpp"


BasePart::BasePart(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID) : 
    Object(model, bt_wrapper, col_shape, mass, baseID){
    m_parent = nullptr;
}


BasePart::BasePart(){
    m_parent = nullptr;
}


BasePart::~BasePart(){
    if(m_parent_constraint.get() != nullptr){
        m_bt_wrapper->removeConstraint(m_parent_constraint.get());
    }
}


BasePart::BasePart(const BasePart& part) : Object(part) {
    m_parent_att_point = part.m_parent_att_point;
    m_attachment_points = part.m_attachment_points;
    m_parent_constraint.reset(nullptr);
    m_parent = nullptr;
}


void BasePart::addAttachmentPoint(const math::vec3& point, const math::vec3& orientation){
    attachment_point att = {point, orientation};
    m_attachment_points.push_back(att);
}


void BasePart::setParentAttachmentPoint(const math::vec3& point, const math::vec3& orientation){
    m_parent_att_point = {point, orientation};
}


const std::vector<struct attachment_point>* BasePart::getAttachmentPoints() const{
    return &m_attachment_points;
}


void BasePart::setParentConstraint(std::unique_ptr<btTypedConstraint>& constraint_uptr){
    if(m_parent_constraint.get() != nullptr){
        m_bt_wrapper->removeConstraint(m_parent_constraint.get());
    }

    m_parent_constraint = std::move(constraint_uptr);
    m_bt_wrapper->addConstraint(m_parent_constraint.get(), true);
}


void BasePart::removeParentConstraint(){
    if(m_parent_constraint.get() != nullptr){
        m_bt_wrapper->removeConstraint(m_parent_constraint.get());
        m_parent_constraint.reset(nullptr);
    }
}


btTypedConstraint* BasePart::getParentConstraint() const{
    return m_parent_constraint.get();
}


const struct attachment_point* BasePart::getParentAttachmentPoint() const{
    return &m_parent_att_point;
}


void BasePart::setParent(BasePart* parent){
    m_parent = parent;
}


bool BasePart::addChild(BasePart* child){
    for(uint i=0; i < m_childs.size(); i++){
        if(m_childs.at(i) == child){
            log("BasePart::addChild - tried to add child part with value ", child, " but it's already in the list");
            std::cerr << "BasePart::addChild - tried to add child part with value " << child << " but it's already in the list" << std::endl;
            return false;
        }
    }
    //std::cout << "part " << child << " has new parent " << this << std::endl;
    m_childs.emplace_back(child);
    return true;
}


bool BasePart::removeChild(BasePart* child){
    for(uint i=0; i < m_childs.size(); i++){
        if(m_childs.at(i) == child){
            m_childs.erase(m_childs.begin() + i);
            //std::cout << "part with value " << this << " has disowned child part with value " << child << std::endl;
            return true;
        }
    }
    log("BasePart::removeChild - tried to remove child part with value ", child, " but it's not in the list");
    std::cerr << "BasePart::removeChild - tried to remove child part with value " << child << " but it's not in the list" << std::endl;
    return false;
}


void BasePart::updateSubTreeMotionState(std::vector<struct set_motion_state_msg>& command_buffer, btVector3 disp, btQuaternion rotation){
    btTransform transform;
    btQuaternion rrotation; // rotation is overrided for now
    btVector3 origin;

    m_body->getMotionState()->getWorldTransform(transform);
    rrotation = transform.getRotation() * rotation;
    origin = transform.getOrigin() + disp;

    command_buffer.emplace_back(set_motion_state_msg{this, origin, rrotation});

    for(uint i=0; i < m_childs.size(); i++){
        m_childs.at(i)->updateSubTreeMotionState(command_buffer, disp, btQuaternion::getIdentity());
    }
}


BasePart* BasePart::getParent() const{
    return m_parent;
}