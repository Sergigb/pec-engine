#include "BasePart.hpp"
#include "Vessel.hpp"
#include "AssetManagerInterface.hpp"


BasePart::BasePart(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager) : 
    Object(model, bt_wrapper, col_shape, mass, baseID){
    m_parent = nullptr;
    m_vessel = nullptr;
    m_is_root = false;
    m_user_rotation = btQuaternion::getIdentity();
    m_has_parent_att = false;
    m_has_free_att = false;
    m_show_editor_menu = false;
    m_asset_manager = asset_manager;
    m_properties = 0;
}


BasePart::BasePart(){
    m_parent = nullptr;
    m_vessel = nullptr;
    m_is_root = false;
    m_user_rotation = btQuaternion::getIdentity();
    m_has_parent_att = false;
    m_has_free_att = false;
    m_show_editor_menu = false;
    m_asset_manager = nullptr;
    m_properties = 0;
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
    m_asset_manager = part.m_asset_manager;
    m_properties = part.m_properties;
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


const btTypedConstraint* BasePart::getParentConstraint() const{
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


const BasePart* BasePart::getParent() const{
    return m_parent;
}


BasePart* BasePart::getParent(){
    return m_parent;
}


std::vector<std::shared_ptr<BasePart>>* BasePart::getChilds(){
    return &m_childs;
}


const std::vector<std::shared_ptr<BasePart>>* BasePart::getChilds() const{
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
        std::stringstream ss;
        ImVec2 mousepos = ImGui::GetMousePos();
        ss << m_unique_id;

        ImGui::SetNextWindowPos(mousepos, ImGuiCond_Appearing);
        ImGui::SetNextWindowSize(ImVec2(300.f, 300.f), ImGuiCond_Appearing);
        ImGui::Begin((m_fancy_name + ss.str()).c_str(), &m_show_editor_menu);

        ImGui::ColorEdit3("Mesh color", m_mesh_color.v);

        ImGui::Checkbox("Set m_has_transform", &m_has_transform);

        ImGui::DragFloat4("1st row", &m_mesh_transform.m[0], 0.1f);
        ImGui::DragFloat4("2nd row", &m_mesh_transform.m[4], 0.1f);
        ImGui::DragFloat4("3rd row", &m_mesh_transform.m[8], 0.1f);
        ImGui::DragFloat4("4th row", &m_mesh_transform.m[12], 0.1f);

        if(ImGui::Button("Set identity")){
            m_mesh_transform = math::identity_mat4();
        }

        // this is for testing now
        if(m_properties & PART_DECOUPLES_CHILDS){
            if(ImGui::Button("Decouple childs")){
                decoupleAll();
            }
        }
        if(m_properties & PART_DECOUPLES){
            if(ImGui::Button("Decouple self")){
                decoupleSelf();
            }
        }
        if(m_properties & PART_HAS_ENGINE){
            if(ImGui::Button("Start engine (does nothing)")){
                // ~~
            }
        }

        ImGui::End();
    }
}


void BasePart::onEditorRightMouseButton(){
    m_show_editor_menu = true;
}


void BasePart::onSimulationRightMouseButton(){
    
}


void BasePart::decoupleAll(){
    for(uint i=0; i < m_childs.size(); i++){
        std::shared_ptr<Vessel> vessel = std::make_shared<Vessel>(m_childs.at(i));
        m_asset_manager->removePartConstraint(m_childs.at(i).get());
        m_asset_manager->addVessel(vessel);
    }
    m_childs.clear();
}


void BasePart::decoupleSelf(){
    std::shared_ptr<BasePart> ourselves = m_vessel->removeChild(this);

    if(ourselves.get() == nullptr){
        std::cerr << "BasePart::decoupleSelf - part with id " << m_unique_id << " tried to decouple itself but owner vessel returned a null pointer" << std::endl;
        log("BasePart::decoupleSelf - part with id ", m_unique_id, " tried to decouple itself but owner vessel returned a null pointer");

        return;
    }

    std::shared_ptr<Vessel> vessel = std::make_shared<Vessel>(ourselves);
    m_asset_manager->removePartConstraint(this);
    m_asset_manager->addVessel(vessel);
}


void BasePart::setProperties(long long int flags){
    m_properties = flags;
}

