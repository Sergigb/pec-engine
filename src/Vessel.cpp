#include "Vessel.hpp"


Vessel::Vessel(){
    m_vessel_root = nullptr;
}


Vessel::Vessel(BasePart* part){
    m_vessel_root = part;
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


void Vessel::setRoot(BasePart* part){
    m_vessel_root = part;
    updateNodes();
}


void Vessel::onChildAppend(std::uint32_t id){
    updateNodes();
    UNUSED(id);

    /*  TO BE COMPLETED
        things that should be done here:
            - update the vessel pointer in each new part (make Vessel a friend class of basePart)
            - something else???

I think the vessel pointer should recursively be updated by the subtree root...

    */
}


BasePart* Vessel::getRoot() const{
    return m_vessel_root;
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
    BasePart* current = m_vessel_root;

    if(current == nullptr){
        return;
    }
    
    m_node_list.clear();
    m_node_map_by_id.clear();

    to_visit.insert(to_visit.end(), current);
    m_node_list.insert(m_node_list.end(), current);
    m_node_map_by_id[current->getUniqueId()] = current;

    while(!to_visit.empty()){
        std::vector<BasePart*>* current_childs = current->getChilds();
        for(uint i=0; i < current_childs->size(); i++){
            m_node_list.insert(m_node_list.end(), current_childs->at(i));
            m_node_map_by_id[current_childs->at(i)->getUniqueId()] = current_childs->at(i);
            to_visit.insert(to_visit.end(), current_childs->at(i));
        }
        current = to_visit.back();
        to_visit.pop_back();
    }
}

