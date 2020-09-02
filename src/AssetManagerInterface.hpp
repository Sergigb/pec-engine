#ifndef ASSET_MANAGER_INTERFACE_HPP
#define ASSET_MANAGER_INTERFACE_HPP

#include <memory>


class Vessel;
class AssetManager;
class BasePart;


class AssetManagerInterface{
    private:
        AssetManager* m_asset_manager;
    public:
        AssetManagerInterface();
        AssetManagerInterface(AssetManager* asset_manager);
        ~AssetManagerInterface();

        void addVessel(std::shared_ptr<Vessel>& vessel);
        void removePartConstraint(BasePart* part);
};


#endif