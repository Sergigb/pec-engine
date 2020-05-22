#include "BasePart.hpp"


BasePart::BasePart(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation, btScalar mass) : 
    Object(model, bt_wrapper, col_shape, origin, local_inertia, initial_rotation, mass){
}


BasePart::BasePart(){

}


BasePart::~BasePart(){

}


void BasePart::addAttachmentPoint(const btVector3& point, const btVector3& orientation){
    attachment_point att = {point, orientation};
    m_attachment_points.push_back(att);
}


void BasePart::setParentAttachmentPoint(const btVector3& point, const btVector3& orientation){
    m_parent_att_point = {point, orientation};
}


const std::vector<struct attachment_point>* BasePart::getAttachmentPoints() const{
    return &m_attachment_points;
}


void BasePart::setParentConstraint(btTypedConstraint* constraint){
    m_parent_constraint.reset(constraint);
}


void BasePart::removeParentConstraint(){
    m_parent_constraint.reset(nullptr);
}


btTypedConstraint* BasePart::getParentConstraint() const{
    return m_parent_constraint.get();
}


const struct attachment_point* BasePart::getParentAttachmentPoint(){
    return &m_parent_att_point;
}

