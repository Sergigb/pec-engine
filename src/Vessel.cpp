#include "Vessel.hpp"


Vessel::Vessel(){
    m_vessel_root = nullptr;
    create_id(m_vessel_id, VESSEL_SET);
}


Vessel::Vessel(std::shared_ptr<BasePart> vessel_root){
    m_vessel_root = vessel_root;
    create_id(m_vessel_id, VESSEL_SET);
    updateNodes();
}


Vessel::~Vessel(){

}


void Vessel::setVesselName(const std::string& name){
    m_vessel_name = name;
}


void Vessel::setVesselDescription(const std::string& description){
    m_vessel_desc = description;
}


/*void Vessel::setRoot(BasePart* part){
    m_vessel_root = part;
    updateNodes();
}*/


void Vessel::onChildAppend(){
    // nothing else for now
    updateNodes();
}


BasePart* Vessel::getRoot() const{
    return m_vessel_root.get();
}


BasePart* Vessel::getPartById(std::uint32_t id) const{
    try{
        return m_node_map_by_id.at(id);
    }
    catch(const std::out_of_range &err){
        return nullptr;
    }
}


const std::string Vessel::getVesselName(){
    return m_vessel_name;
}


const std::string Vessel::getVesselDescription(){
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
        std::vector<std::shared_ptr<BasePart>>* current_childs = current->getChilds();
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


std::uint32_t Vessel::getId() const{
    return m_vessel_id;
}

