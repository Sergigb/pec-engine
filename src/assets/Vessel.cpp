#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "Vessel.hpp"
#include "BasePart.hpp"
#include "../core/Player.hpp"
#include "../core/Physics.hpp" // collision flags
#include "../core/id_manager.hpp"
#include "../core/log.hpp"
#include "../core/Input.hpp"


Vessel::Vessel(){
    m_vessel_root = nullptr;
    create_id(m_vessel_id, VESSEL_SET);
    m_player = nullptr;
    m_input = nullptr;
    m_yaw = 0.0f;
    m_pitch = 0.0f;
    m_total_mass = 0.0;
    m_com = btVector3(0.0, 0.0, 0.0);
}


Vessel::Vessel(std::shared_ptr<BasePart>& vessel_root, const Input* input){
    m_vessel_root = vessel_root;
    m_vessel_root->setRoot(true);
    m_vessel_root->updateSubTreeVessel(this);
    create_id(m_vessel_id, VESSEL_SET);
    m_player = nullptr;
    m_input = input;
    m_yaw = 0.0f;
    m_pitch = 0.0f;
    m_com = btVector3(0.0, 0.0, 0.0);

    updateNodes();
    updateMass();
}


Vessel::~Vessel(){
    if(m_player){
        m_player->onVesselDestroy();
    }
}


void Vessel::setVesselName(const std::string& name){
    m_vessel_name = name;
}


void Vessel::setVesselDescription(const std::string& description){
    m_vessel_desc = description;
}


/*void Vessel::setRoot(BasePart* part){
    m_vessel_root = part;
    m_vessel_root->setRoot(true);
    updateNodes();
}*/


void Vessel::onTreeUpdate(){
    updateNodes();
    updateMass();
    updateStaging();
}


BasePart* Vessel::getRoot(){
    return m_vessel_root.get();
}


BasePart* Vessel::getPartById(std::uint32_t id){
    try{
        return m_node_map_by_id.at(id);
    }
    catch(const std::out_of_range &err){
        return nullptr;
    }
}


const BasePart* Vessel::getRoot() const{
    return m_vessel_root.get();
}


const BasePart* Vessel::getPartById(std::uint32_t id) const{
    try{
        return m_node_map_by_id.at(id);
    }
    catch(const std::out_of_range &err){
        return nullptr;
    }
}


const std::string Vessel::getVesselName() const{
    return m_vessel_name;
}


const std::string Vessel::getVesselDescription() const{
    return m_vessel_desc;
}


void Vessel::updateNodes(){
    std::vector<BasePart *> to_visit;
    BasePart* current = m_vessel_root.get();

    if(current == nullptr){
        return;
    }
    
    m_node_list.clear();
    m_node_map_by_id.clear();

    to_visit.insert(to_visit.end(), current);
    m_node_list.insert(m_node_list.end(), current);
    m_node_map_by_id[current->getUniqueId()] = current;

    while(!to_visit.empty()){
        const std::vector<std::shared_ptr<BasePart>>* current_childs = current->getChilds();
        for(uint i=0; i < current_childs->size(); i++){
            m_node_list.insert(m_node_list.end(), current_childs->at(i).get());
            m_node_map_by_id[current_childs->at(i)->getUniqueId()] = current_childs->at(i).get();
            to_visit.insert(to_visit.end(), current_childs->at(i).get());
        }
        current = to_visit.back();
        to_visit.pop_back();
    }
}


std::vector<BasePart*>* Vessel::getParts(){
    return &m_node_list;
}


const std::vector<BasePart*>* Vessel::getParts() const{
    return &m_node_list;
}


std::uint32_t Vessel::getId() const{
    return m_vessel_id;
}


const std::string vertical_and_right = "\u251C";
const std::string horizontal = "\u2500\u2500\u2500";
const std::string vertical = "\u2502";
const std::string up_and_right = "\u2514";
const std::string separator = "    ";

void print_tree_member(BasePart *node, std::string tail, bool last_child){
    std::string part_name;
    node->getFancyName(part_name);

    std::string next_tail;
    if(!last_child){
        std::cout << tail << up_and_right  << horizontal << " " << part_name << " - " << node->getUniqueId();

        if(node->getClones().size()){
            std::cout << " - Clones: ";
            for(uint i=0; i < node->getClones().size(); i++){
                std::cout << node->getClones().at(i)->getUniqueId() << ", ";
            }
        }
        if(node->getClonedFrom()){
            std::cout << " - Node cloned from: " << node->getClonedFrom()->getUniqueId();
        }
        std::cout << std::endl;

        next_tail = tail + " " + separator;
    }
    else{
        std::cout << tail << vertical_and_right  << horizontal << " " << part_name << " - " << node->getUniqueId();

        if(node->getClones().size()){
            std::cout << " - Clones: ";
            for(uint i=0; i < node->getClones().size(); i++){
                std::cout << node->getClones().at(i)->getUniqueId() << ", ";
            }
        }
        if(node->getClonedFrom()){
            std::cout << " - Node cloned from: " << node->getClonedFrom()->getUniqueId();
        }
        std::cout << std::endl;

        next_tail = tail + vertical + separator;
    }

    for(uint i=0; i < node->getChilds()->size(); i++)
        print_tree_member(node->getChilds()->at(i).get(), next_tail, !(i == node->getChilds()->size() - 1));
}


void Vessel::printVessel() const{
    if(m_vessel_root == nullptr){
        std::cerr << "Ship::printTree - ship has no part tree" << std::endl;
        log("Ship::printTree - ship has no part tree");
        return;
    }
    std::string tail = "";
    std::cout << m_vessel_name << "'s part tree:" << std::endl;
    print_tree_member(m_vessel_root.get(), tail, false);
}


void Vessel::printStaging() const{
    std::cout << std::endl;
    for(uint i=0; i < m_stages.size(); i++){
        std::vector<stage_action> v = m_stages.at(i);
        std::cout << "Stage " << i + 1 << std::endl;
        for(uint j=0; j < v.size(); j++){
            std::string name;
            v.at(j).part->getFancyName(name);
            std::cout << "\t" << name << " - " << v.at(j).part->getUniqueId() << std::endl;
        }
    }
}


bool Vessel::addChildById(std::shared_ptr<BasePart>& child, std::uint32_t parent_id){
    try{
        BasePart* parent = m_node_map_by_id.at(parent_id);
        child->setParent(parent);
        child->updateSubTreeVessel(this);
        child->setCollisionMaskSubTree(child->m_body->getBroadphaseProxy()->m_collisionFilterMask | CG_RAY_EDITOR_RADIAL);
        parent->addChild(child);
        onTreeUpdate();

        return true;
    }
    catch(const std::out_of_range& oor){
        std::cerr << "Vessel::addChildById - could not add child because the parent ID " << parent_id 
                  << " does not belong to this vessel (" << m_vessel_id << " - " << oor.what() << std::endl;    
        log("Vessel::addChildById - could not add child because the parent ID ", parent_id, 
            " does not belong to this vessel (", m_vessel_id, ") - ", oor.what());

        return false;
    }
}


bool Vessel::addChild(BasePart* child, BasePart* parent){
    if(parent->getVessel()->getId() != m_vessel_id){
        std::cerr << "Vessel::addChild - could not add child because the parent (" << parent->getUniqueId() 
                  << ") does not belong to this vessel (" << m_vessel_id << ")" << std::endl;
        log("Vessel::addChild - could not add child because the parent (", parent->getUniqueId(),
            ") does not belong to this vessel (", m_vessel_id, ")");
        return false;
    }
    std::shared_ptr<BasePart> child_sptr = std::dynamic_pointer_cast<BasePart>(child->getSharedPtr());

    child->setParent(parent);
    child->updateSubTreeVessel(this);
    child->setCollisionMaskSubTree(child->m_body->getBroadphaseProxy()->m_collisionFilterMask | CG_RAY_EDITOR_RADIAL);
    parent->addChild(child_sptr);
    onTreeUpdate();

    return true;
}


std::shared_ptr<BasePart> Vessel::removeChildById(std::uint32_t child_id){
    std::shared_ptr<BasePart> child_sptr;
    try{
        BasePart* child = m_node_map_by_id.at(child_id);

        if(child->isRoot()){
            std::cerr << "Vessel::removeChild - child with id " << child->getUniqueId()
                      << " is the root and can not be removed" << std::endl;
            log("Vessel::removeChild - child with id ", child->getUniqueId(),
                " is the root and can not be removed");
            return child_sptr;
        }

        BasePart* parent = child->getParent();

        child->setParent(nullptr);
        child->updateSubTreeVessel(nullptr);
        child->setCollisionMaskSubTree(child->m_body->getBroadphaseProxy()->m_collisionFilterMask & ~CG_RAY_EDITOR_RADIAL);
        child_sptr = parent->removeChild(child);
        onTreeUpdate();

        return child_sptr;
    }
    catch(const std::out_of_range& oor){
        std::cerr << "Vessel::removeChildById - could not remove child with id " << child_id 
                  << " because it does not belong to this vessel (" << m_vessel_id << ") - " << oor.what() << std::endl;    
        log("Vessel::removeChildById - could not remove child with id ", child_id,
            " because it does not belong to this vessel (", m_vessel_id, ") - ", oor.what());

        return child_sptr;
    }
}


std::shared_ptr<BasePart> Vessel::removeChild(BasePart* child){
    std::shared_ptr<BasePart> child_sptr;
    if(child->getVessel()->getId() != m_vessel_id){
        std::cerr << "Vessel::removeChild - could not remove child with id " << child->getUniqueId()
                  << " as it does not belong to this vessel (" << m_vessel_id << ")" << std::endl;
        log("Vessel::removeChild - could not remove child with id ", child->getUniqueId(),
            " as it does not belong to this vessel (", m_vessel_id, ")");
        return child_sptr;
    }
    if(child->isRoot()){
        std::cerr << "Vessel::removeChild - child with id " << child->getUniqueId()
                  << " is the root and can not be removed" << std::endl;
        log("Vessel::removeChild - child with id ", child->getUniqueId(),
            " is the root and can not be removed");
        return child_sptr;
    }
    BasePart* parent = child->getParent();

    child->setParent(nullptr);
    child->updateSubTreeVessel(nullptr);
    child->setCollisionMaskSubTree(child->m_body->getBroadphaseProxy()->m_collisionFilterMask & ~CG_RAY_EDITOR_RADIAL);
    child_sptr = parent->removeChild(child);
    onTreeUpdate();

    return child_sptr;                
}


void Vessel::setPlayer(Player* player){
    m_player = player;
}


void Vessel::update(){
    if(m_player){
        if(m_input->pressed_keys[GLFW_KEY_W] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
            m_yaw = 1.0f;
        }
        else if(m_input->pressed_keys[GLFW_KEY_S] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
            m_yaw = -1.0f;
        }
        else{
            m_yaw = 0.0f;
        }
        if(m_input->pressed_keys[GLFW_KEY_A] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
            m_pitch = 1.0f;
        }
        else if(m_input->pressed_keys[GLFW_KEY_D] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){
            m_pitch = -1.0f;
        }
        else{
            m_pitch = 0.0f;
        }
    }

    // engage current active stage
    if(m_input->pressed_keys[GLFW_KEY_SPACE] & INPUT_KEY_DOWN){
        if(m_stages.size() > 0){
            activateNextStage();
        }
    }

    for(uint i=0; i < m_node_list.size(); i++){
        m_node_list.at(i)->update();
    }

    updateMass();
}


const Input* Vessel::getInput() const{
    return m_input;
}


float Vessel::getYaw() const{
    return m_yaw;
}


float Vessel::getPitch() const{
    return m_pitch;
}


void Vessel::updateMass(){
    double temp_mass = 0.0;
    for(uint i=0; i < m_node_list.size(); i++){
         temp_mass += m_node_list.at(i)->getMass();
    }
    m_total_mass = temp_mass;
}


double Vessel::getTotalMass() const{
    return m_total_mass;
}


void Vessel::setVesselVelocity(const btVector3& velocity){
    m_vessel_root->setSubTreeVelocity(velocity);
}


void Vessel::updateCoM(){
    btVector3 com(0.0, 0.0, 0.0);

    for(uint i=0; i < m_node_list.size(); i++){
        if(!m_node_list.at(i)->m_body.get()) continue;

        btTransform trans;
        m_node_list.at(i)->m_body->getMotionState()->getWorldTransform(trans);
        const btVector3& origin = trans.getOrigin();

        com += (m_node_list.at(i)->getMass() / m_total_mass) * origin;
    }
    m_com = com;
}


const btVector3& Vessel::getCoM() const{
    return m_com;
}


void Vessel::updateStaging(){
    m_stages.clear();
    updateStagingRec(m_vessel_root.get(), 1);
    if(m_stages.back().size() == 0){
        m_stages.pop_back();
    }
}


void Vessel::updateStagingRec(BasePart* part, int stage){
    // stage starts at 1
    long properties = part->getProperties();
    
    if((int)m_stages.size() < stage){
        std::vector<stage_action> s;
        m_stages.emplace_back(s);
    }

    // maybe we could replace these ifs with a method on part that asks for the default action to take (only one?)
    if(properties & PART_SEPARATES){
        m_stages.at(stage - 1).emplace_back(part, PART_ACTION_SEPARATE);
    }
    if(properties & PART_HAS_ENGINE){
        m_stages.at(stage - 1).emplace_back(part, PART_ACTION_ENGINE_START);
    }

    if(properties & PART_SEPARATES){
        stage++;
    }

    std::vector<std::shared_ptr<BasePart>>* childs = part->getChilds();
    for(uint i=0; i < childs->size(); i++){
        updateStagingRec(childs->at(i).get(), stage);
    }
}


void Vessel::activateNextStage(){
    // decoupling parts need to be stored in a temporary vector because after a decoupling the staging gets updated
    std::vector<BasePart*> to_decouple;
    std::vector<stage_action>& stage = m_stages.back();

    for(uint i=0; i < stage.size(); i++){
        stage_action& current_action = stage.at(i);
        if(current_action.action == PART_ACTION_SEPARATE){
            to_decouple.emplace_back(current_action.part);
        }
        else{
            current_action.part->action(current_action.action);
        }
    }

    for(uint i=0; i < to_decouple.size(); i++){
        to_decouple.at(i)->action(PART_ACTION_SEPARATE);
    }

    m_stages.pop_back();
}

