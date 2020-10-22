#ifndef ASSET_MANAGER_INTERFACE_HPP
#define ASSET_MANAGER_INTERFACE_HPP

#include <memory>


class Vessel;
class AssetManager;
class BasePart;

struct apply_force_msg;


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
};


#endif