#ifndef ASSET_MANAGER_INTERFACE_HPP
#define ASSET_MANAGER_INTERFACE_HPP

#include <memory>
#include <vector>


class Vessel;
class AssetManager;
class BasePart;
class btTypedConstraint;
class Object;
class Model;


/*
 * This class is used as an interface between the asset manager and the vessels/parts, this is
 * mainly used to add commands to the command buffers. Command buffers are usually used by the
 * update methods, as the physics thread is most likely stepping. Command buffers can be used,
 * for example, to change the motion state of a part or to add/remove constraints. The commands in
 * the buffers are applied before the new engine step, but NOT durnig the current one. This class
 * is inherited by AssetManager.
 */

class AssetManagerInterface{
    protected:
        /* command buffers */
        std::vector<struct set_motion_state_msg> m_set_motion_state_buffer;
        std::vector<BasePart*> m_remove_part_constraint_buffer;
        std::vector<struct add_contraint_msg> m_add_constraint_buffer;
        std::vector<struct add_body_msg> m_add_body_buffer;
        std::vector<std::shared_ptr<Vessel>> m_add_vessel_buffer;
        std::vector<struct apply_force_msg> m_apply_force_buffer;
        std::vector<struct set_mass_props_msg> m_set_mass_buffer;
        std::vector<std::shared_ptr<BasePart>> m_delete_subtree_buffer;
        std::vector<BasePart*> m_build_constraint_subtree_buffer;
        std::vector<struct set_vessel_velocity_msg> m_set_vessel_velocity_buffer;
        std::vector<std::unique_ptr<Model>> m_public_models;
    public:
        AssetManagerInterface();
        ~AssetManagerInterface();

        /* 
         * Adds the vessel to m_active_vessels. I honestly don't think this buffer is necessary,
         * as adding a vessel to m_active_vessels shouldn't involve the physics thread at all. The
         * only reason I can think of is that we might be iterating over m_active_vessels when we
         * add the new vessel, but I still think it doesn't matter.
         * 
         * @vessel: rvalue reference to the unique ptr of the vessel, the buffer will take 
         * ownership of this pointer.
         */
        void addVessel(std::shared_ptr<Vessel>&& vessel);

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
         * @c_uptr: rvalue reference to the unique pointer to the constraint. The buffer message 
         * takes ownership of the pointer.
         */
        void addConstraint(BasePart* ptr, std::unique_ptr<btTypedConstraint>&& c_uptr);

        /* 
         * Adds parts to the m_build_constraint_subtree_buffer. All the parts in this buffer will
         * have the constraints of their subtrees built by recursively calling the parts
         * buildSubTreeConstraints method.
         *
         * This method should be used when a subtree is cloned (for example in the editor) or when
         * a vessel is spawned in the middle of the simulation, as the constraints can't be added
         * to the dynamics world because Bullet may be stepping.
         *
         * @part: pointer to the root part of the subtree.
         */
        void buildConstraintSubtree(BasePart* part);

        /*
         * Adds the subtree under root in the delete subtree buffer. The whole subtree will be 
         * deleted, this includes removing the rigid bodies from the dynamics world.
         *
         * @root: rvalue reference of the shared pointer to the root of the tree, this buffer will
         * take ownership of this pointer and the rest of the subtree.
         */
        void deleteSubtree(std::shared_ptr<BasePart>&& root);

        /*
         * Sets the motion state (rotation and origin) of the object.
         *
         * @obj: raw pointer to the object.
         * @orig: btVector3 with the new origin of the object.
         * @rot: btQuaternion with the new rotation of the object.
         */
        void setMotionState(Object* obj, const btVector3& orig, const btQuaternion& rot);

        /*
         * Sets the velocity of the subtree of a vessel.
         *
         * @vessel: raw pointer to a Vessel.
         * @vel: new velocity value.
         */
        void setVesselVelocity(Vessel* vessel, const btVector3& vel);

        /*
         * Stores a model in the m_public_models' vector. Useful if you want to store from model
         * just once.
         *
         * @model: rvalue ref to the shared pointer of the model, which this class will take
         * ownership of.
         */
        void storeModel(std::unique_ptr<Model>&& model);
};


#endif