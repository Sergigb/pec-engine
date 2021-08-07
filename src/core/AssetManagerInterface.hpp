#ifndef ASSET_MANAGER_INTERFACE_HPP
#define ASSET_MANAGER_INTERFACE_HPP

#include <memory>


class Vessel;
class AssetManager;
class BasePart;
class btTypedConstraint;


/*
 * This class is used as an interface between the asset manager and the vessels/parts, this is
 * mainly used to add commands to the command buffers, usually from the update methods, as the
 * physics thread is most likely stepping.
 */

class AssetManagerInterface{
    private:
        AssetManager* m_asset_manager;
    public:
        AssetManagerInterface();
        AssetManagerInterface(AssetManager* asset_manager);
        ~AssetManagerInterface();

        /* 
         * Adds the vessel to m_active_vessels. I honestly don't think this buffer is necessary,
         * as adding a vessel to m_active_vessels shouldn't involve the physics thread at all. The
         * only reason I can think of is that we might be iterating over m_active_vessels when we
         * add the new vessel, but I still think it doesn't matter.
         * 
         * @vessel: unique ptr of the vessel, the buffer will take ownership of this pointer.
         */
        void addVessel(std::shared_ptr<Vessel>& vessel);

        /*
         * Removes the constraint of the part. Note that constraints are owned by the childs, so
         * this will "physically" detach this part from its parent. Note also that this method does
         * not logically detach the child from its parent, this should be done separately.
         *
         * @part: pointer to the part we want the constraint removed from.
         */
        void removePartConstraint(BasePart* part);

        /*
         * Applies a force to the rigid body of a part given a force vector and the center of the
         * force.
         *
         * @ptr: pointer to the part we want to apply the force to.
         * @f: force vector, in newtons I guess.
         * @r_pos: relative application point of the force, in meters.
         */
        void applyForce(BasePart* ptr, const btVector3& f, const btVector3& r_pos);

        /*
         * Sets the mass of a part. I don't know what "Props" is supposed to mean.
         *
         * @ptr: pointer to the part.
         * @m: new mass value, in kgs.
         */
        void setMassProps(BasePart* ptr, double m);

        /* 
         * Adds the rigid body of the part to the dynamics world, useful when new things are spawned.
         *
         * @ptr: pointer to the new part to be added.
         * @orig: origin of the part.
         * @iner: inertia vector.
         * @rot: rotation of the part.
         */
        void addBody(BasePart* ptr, const btVector3& orig, const btVector3& iner, const btQuaternion& rot);

        /*
         * Adds a constraint to the part, note that the part should already be in the dynamics 
         * world. The buffer will take ownership of the constraint, which will eventually be
         * passed to the part.
         *
         * @ptr: pointer to the part we want to add the constraint to.
         * @c_uptr: unique pointer to constraint.
         */
        void addConstraint(BasePart* ptr, std::unique_ptr<btTypedConstraint>& c_uptr);

        /* 
         * Adds parts to the m_build_constraint_subtree_buffer. All the parts in this buffer will
         * have the constraints of their subtrees built by calling the part's buildSubTreeConstraints
         * method.
         *
         * This method should be used when a subtree is cloned (for example in the editor) or when
         * a vessel is spawned in the middle of the simulation, as the constraints can't be added
         * to the dynamics world because Bullet may be stepping.
         *
         * @part: pointer to the root part of the subtree.
         */
        void buildConstraintSubtree(BasePart* part);
};


#endif