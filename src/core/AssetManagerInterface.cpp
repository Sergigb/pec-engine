#include <iostream>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "AssetManagerInterface.hpp"
#include "AssetManager.hpp"


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


void AssetManagerInterface::applyForce(BasePart* ptr, const btVector3& f, const btVector3& r_pos){
    m_asset_manager->m_apply_force_buffer.emplace_back(ptr, f, r_pos);
}


void AssetManagerInterface::setMassProps(BasePart* ptr, double m){
    m_asset_manager->m_set_mass_buffer.emplace_back(ptr, m);
}


void AssetManagerInterface::addBody(BasePart* ptr, const btVector3& orig, const btVector3& iner, const btQuaternion& rot){
    m_asset_manager->m_add_body_buffer.emplace_back(ptr, orig, iner, rot);
}


void AssetManagerInterface::addConstraint(BasePart* ptr, std::unique_ptr<btTypedConstraint>& c_uptr){
    m_asset_manager->m_add_constraint_buffer.emplace_back(ptr, c_uptr);
}


void AssetManagerInterface::buildConstraintSubtree(BasePart* part){
    m_asset_manager->m_build_constraint_subtree_buffer.emplace_back(part);
}

