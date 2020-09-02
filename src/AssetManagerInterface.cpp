#include "AssetManagerInterface.hpp"
#include "AssetManager.hpp"
#include <iostream>


AssetManagerInterface::AssetManagerInterface(){

}


AssetManagerInterface::AssetManagerInterface(AssetManager* asset_manager){
    m_asset_manager = asset_manager;
}


AssetManagerInterface::~AssetManagerInterface(){

}


void AssetManagerInterface::addVessel(std::shared_ptr<Vessel>& vessel){
    m_asset_manager->m_add_vessel_buffer.emplace_back(vessel);
}


void AssetManagerInterface::removePartConstraint(BasePart* part){
    m_asset_manager->m_remove_part_constraint_buffer.emplace_back(part);

}
