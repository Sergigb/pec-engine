#ifndef ASSET_MANAGER_INTERFACE_HPP
#define ASSET_MANAGER_INTERFACE_HPP

#include <memory>


class Vessel;
class AssetManager;
class BasePart;
class btTypedConstraint;


class AssetManagerInterface{
    private:
        AssetManager* m_asset_manager;
    public:
        AssetManagerInterface();
        AssetManagerInterface(AssetManager* asset_manager);
        ~AssetManagerInterface();

        void addVessel(std::shared_ptr<Vessel>& vessel);
        void removePartConstraint(BasePart* part);
        void applyForce(BasePart* ptr, const btVector3& f, const btVector3& r_pos);
        void setMassProps(BasePart* ptr, double m);
        void addBody(BasePart* ptr, const btVector3& orig, const btVector3& iner, const btQuaternion& rot);
        void addConstraint(BasePart* ptr, std::unique_ptr<btTypedConstraint>& c_uptr);
        void buildConstraintSubtree(BasePart* part);
};


#endif