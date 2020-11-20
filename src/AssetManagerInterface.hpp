#ifndef ASSET_MANAGER_INTERFACE_HPP
#define ASSET_MANAGER_INTERFACE_HPP

#include <memory>


class Vessel;
class AssetManager;
class BasePart;

struct apply_force_msg;
struct set_mass_props_msg;
struct add_body_msg;
struct add_contraint_msg;


class AssetManagerInterface{
    private:
        AssetManager* m_asset_manager;
    public:
        AssetManagerInterface();
        AssetManagerInterface(AssetManager* asset_manager);
        ~AssetManagerInterface();

        void addVessel(std::shared_ptr<Vessel>& vessel);
        void removePartConstraint(BasePart* part);
        void applyForce(apply_force_msg& msg);
        void setMassProps(set_mass_props_msg& msg);
        void addBody(const add_body_msg& msg);
        void addConstraint(add_contraint_msg& msg);
        void buildConstraintSubtree(BasePart* part);
};


#endif