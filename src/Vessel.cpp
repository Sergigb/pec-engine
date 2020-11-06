#include "Vessel.hpp"
#include "Player.hpp"
#include "BtWrapper.hpp" // collision flags
#include "BasePart.hpp"
#include "id_manager.hpp"
#include "log.hpp"
#include "Input.hpp"


Vessel::Vessel(){
    m_vessel_root = nullptr;
    create_id(m_vessel_id, VESSEL_SET);
    m_player = nullptr;
    m_input = nullptr;
}


Vessel::Vessel(std::shared_ptr<BasePart>& vessel_root, const Input* input){
    m_vessel_root = vessel_root;
    m_vessel_root->setRoot(true);
    m_vessel_root->updateSubTreeVessel(this);
    create_id(m_vessel_id, VESSEL_SET);
    updateNodes();
    m_player = nullptr;
    m_input = input;
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
    // nothing else for now
    updateNodes();
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
        std::cout << tail << up_and_right  << horizontal << " " << part_name << " - " << node->getUniqueId() << std::endl;
        next_tail = tail + " " + separator;
    }
    else{
        std::cout << tail << vertical_and_right  << horizontal << " " << part_name << " - " << node->getUniqueId() << std::endl;
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


bool Vessel::addChildById(std::shared_ptr<BasePart>& child, std::uint32_t parent_id){
    try{
        BasePart* parent = m_node_map_by_id.at(parent_id);
        child->setParent(parent);
        child->updateSubTreeVessel(this);
        child->setCollisionMaskSubTree(child->m_body->getBroadphaseProxy()->m_collisionFilterMask | CG_RAY_EDITOR_RADIAL);
        parent->addChild(child);
        updateNodes();

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
    updateNodes();

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
        updateNodes();

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
    updateNodes();

    return child_sptr;                
}


void Vessel::setPlayer(Player* player){
    m_player = player;
}


void Vessel::update(){
    for(uint i=0; i < m_node_list.size(); i++){
        m_node_list.at(i)->update();
    }
}


const Input* Vessel::getInput() const{
    return m_input;
}

