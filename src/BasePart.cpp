#include "BasePart.hpp"


BasePart::BasePart(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID) : 
    Object(model, bt_wrapper, col_shape, mass, baseID){
}


BasePart::BasePart(){

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

