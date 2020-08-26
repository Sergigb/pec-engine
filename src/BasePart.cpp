#include "BasePart.hpp"


BasePart::BasePart(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID) : 
    Object(model, bt_wrapper, col_shape, mass, baseID){
    m_parent = nullptr;
    m_vessel = nullptr;
    m_is_root = false;
    m_user_rotation = btQuaternion::getIdentity();
    m_has_parent_att = false;
    m_has_free_att = false;
    m_show_editor_menu = false;
}


BasePart::BasePart(){
    m_parent = nullptr;
    m_vessel = nullptr;
    m_is_root = false;
    m_user_rotation = btQuaternion::getIdentity();
    m_has_parent_att = false;
    m_has_free_att = false;
    m_show_editor_menu = false;
}


BasePart::~BasePart(){
    if(m_parent_constraint.get() != nullptr){
        m_bt_wrapper->removeConstraint(m_parent_constraint.get());
    }
}


BasePart::BasePart(const BasePart& part) : Object(part) {
    m_parent_att_point = part.m_parent_att_point;
    m_attachment_points = part.m_attachment_points;
    m_free_att_point = part.m_free_att_point;
    m_parent_constraint.reset(nullptr);
    m_parent = nullptr;
    m_is_root = false;
    m_vessel = nullptr;
    m_user_rotation = btQuaternion::getIdentity();
    m_has_parent_att = part.m_has_parent_att;
    m_has_free_att = part.m_has_free_att;
    m_show_editor_menu = false;
}


void BasePart::addAttachmentPoint(const math::vec3& point, const math::vec3& orientation){
    attachment_point att = {point, orientation};
    m_attachment_points.push_back(att);
}


void BasePart::setParentAttachmentPoint(const math::vec3& point, const math::vec3& orientation){
    m_has_parent_att = true;
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


bool BasePart::addChild(std::shared_ptr<BasePart>& child){
    for(uint i=0; i < m_childs.size(); i++){
        if(m_childs.at(i).get() == child.get()){
            log("BasePart::addChild - tried to add child part with value ", child.get(), " but it's already in the list");
            std::cerr << "BasePart::addChild - tried to add child part with value " << child.get() << " but it's already in the list" << std::endl;
            return false;
        }
    }
    //std::cout << "part " << child << " has new parent " << this << std::endl;
    m_childs.emplace_back(child);
    return true;
}


std::shared_ptr<BasePart> BasePart::removeChild(BasePart* child){
    std::shared_ptr<BasePart> partsptr(nullptr);
    for(uint i=0; i < m_childs.size(); i++){
        if(m_childs.at(i).get() == child){
            partsptr = std::dynamic_pointer_cast<BasePart>(m_childs.at(i)); 
            m_childs.erase(m_childs.begin() + i);
            //std::cout << "part with value " << this << " has disowned child part with value " << child << std::endl;
            return partsptr;
        }
    }
    log("BasePart::removeChild - tried to remove child part with value ", child, " but it's not in the list");
    std::cerr << "BasePart::removeChild - tried to remove child part with value " << child << " but it's not in the list" << std::endl;
    return partsptr;
}


void BasePart::updateSubTreeMotionState(std::vector<struct set_motion_state_msg>& command_buffer, btVector3 disp, btVector3 root_origin, btQuaternion rotation){
    btTransform transform, trans, trans_r;
    btQuaternion rrotation;
    btVector3 origin, dist_from_root;

    m_body->getMotionState()->getWorldTransform(transform);
    rrotation = transform.getRotation();
    origin = transform.getOrigin();

    dist_from_root = origin - root_origin;
    trans = btTransform(rrotation, dist_from_root);
    trans_r = btTransform(rotation, btVector3(0.0, 0.0, 0.0));
    trans = trans_r * trans;

    command_buffer.emplace_back(set_motion_state_msg{this, root_origin + trans.getOrigin() + disp, trans.getRotation()});

    for(uint i=0; i < m_childs.size(); i++){
        m_childs.at(i)->updateSubTreeMotionState(command_buffer, disp, root_origin, rotation);
    }
}


BasePart* BasePart::getParent() const{
    return m_parent;
}


const std::vector<std::shared_ptr<BasePart>>* BasePart::getChilds(){
    return &m_childs;
}


const Vessel* BasePart::getVessel() const{
    return m_vessel;
}


void BasePart::updateSubTreeVessel(Vessel* vessel){
    m_vessel = vessel;

    for(uint i=0; i < m_childs.size(); i++){
        m_childs.at(i)->updateSubTreeVessel(vessel);
    }
}


int BasePart::render(){
    math::mat4 body_transform = getRigidBodyTransformSingle();
 
    if(m_has_transform){
        body_transform = body_transform * m_mesh_transform;
    }

    if(m_vessel){
        m_model->setMeshColor(math::vec4(m_mesh_color, 1.0));
    }
    else{
        m_model->setMeshColor(math::vec4(m_mesh_color, 0.5));
    }
    
    return m_model->render(body_transform);
}


int BasePart::render(math::mat4 body_transform){
    if(m_has_transform){
        body_transform = body_transform * m_mesh_transform;
    }

    if(m_vessel){
        m_model->setMeshColor(math::vec4(m_mesh_color, 1.0));
    }
    else{
        m_model->setMeshColor(math::vec4(m_mesh_color, 0.5));
    }

    return m_model->render(body_transform);
}



void BasePart::setRenderIgnoreSubTree(){
    m_render_ignore = true;

    for(uint i=0; i < m_childs.size(); i++){
        m_childs.at(i)->setRenderIgnoreSubTree();
    }
}


void BasePart::setRoot(bool root){
    m_is_root = root;
}


bool BasePart::isRoot() const{
    return m_is_root;
}


void BasePart::setCollisionMaskSubTree(short mask){
    m_body->getBroadphaseProxy()->m_collisionFilterMask = mask;

    for(uint i=0; i < m_childs.size(); i++){
        m_childs.at(i)->setCollisionMaskSubTree(mask);
    }
}


void BasePart::setFreeAttachmentPoint(const math::vec3& point, const math::vec3& orientation){
    m_has_free_att = true;
    m_free_att_point = {point, orientation};
}


const struct attachment_point* BasePart::getFreeAttachmentPoint() const{
    return &m_free_att_point;
}


bool BasePart::hasParentAttPoint() const{
    return m_has_parent_att;
}


bool BasePart::hasFreeAttPoint() const{
    return m_has_free_att;
}


void BasePart::renderOther(){
    if(m_show_editor_menu){
        ImVec2 mousepos = ImGui::GetMousePos();

        ImGui::SetNextWindowPos(mousepos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(300.f, 200.f), ImGuiCond_Appearing);
        ImGui::Begin(m_fancy_name.c_str(), &m_show_editor_menu);

        ImGui::ColorEdit3("Mesh color", m_mesh_color.v);

        ImGui::End();
    }
}


void BasePart::onEditorRightMouseButton(){
    m_show_editor_menu = true;
}

