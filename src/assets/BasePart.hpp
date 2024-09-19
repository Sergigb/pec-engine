#ifndef BASEPART_HPP
#define BASEPART_HPP

#include <vector>
#include <memory>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>
#include <imgui.h>

#include "Object.hpp"
#include "../core/maths_funcs.hpp"


class Vessel;
class AssetManagerInterface;
class Resource;
class Physics;
class Model;
class EngineComponent;

namespace tinyxml2{
    class XMLElement;
}

struct object_transform;


/*
 * Describes an attachment point of the part.
 *
 * @point: local origin of the attachment.
 * @orientation: orientation of the attachment point.
 */
struct attachment_point{
    math::vec3 point;
    math::vec3 orientation;
};


/*
 * Describes a container of a certain resource. The volume of the container can be derived from 
 * density of the resource, if needed.
 *
 * @resource: pointer to the resource object.
 * @mass: current mass inside the container, in kilograms.
 * @mass_max: maximum capacity of the container, in kilograms.
 */
struct resource_container{
    Resource* resource;
    float mass;
    float max_mass;
};



/* basic properties flags */
#define PART_SEPARATES 1
#define PART_IS_CM 2
#define PART_HAS_ENGINE 4


/* generic actions */
#define PART_ACTION_SEPARATE 1
#define PART_ACTION_ENGINE_TOGGLE 2
#define PART_ACTION_ENGINE_START 3


/*
 * Base class for parts, vessels are a tree made of parts. To create parts with extra
 * functionalities, this class can be derived. This class can be used for multiple parts, such as
 * fuel tanks, wings, structural elements or others.
 *
 * When the master part's list is ceated, we call the constructor of this class normally but in the
 * editor we call the copy constructor of the master part or, in case of cloning subtrees, we clone
 * the parts of the subtree.
 */
class BasePart : public Object{
    protected:
        struct attachment_point m_parent_att_point, m_free_att_point;
        std::vector<struct attachment_point> m_attachment_points;  // the attachment points of the part
        std::unique_ptr<btTypedConstraint> m_parent_constraint; // the constraint between this part and the parent
        BasePart* m_parent;
        std::vector<std::shared_ptr<BasePart>> m_childs;
        Vessel* m_vessel;
        bool m_is_root, m_has_parent_att, m_has_free_att;
        bool m_show_editor_menu, m_show_game_menu;
        long m_properties;
        float m_alpha;

        /* editor cloning stuff, shouldn't be used outside of the editor */
        std::vector<BasePart*> m_clones;
        BasePart* m_cloned_from;

        // components of the part
        std::vector<struct resource_container> m_resources;
        std::vector<EngineComponent*> m_engine_list;

        AssetManagerInterface* m_asset_manager;

        btVector3 m_velocity, m_prev_velocity;  // remove me later

        /*
         * Decouples all the childs of this part.
         */
        void decoupleChilds();
        
        /*
         * Decouples itself from its parent.
         */
        void decoupleSelf();

        /*
         * Called upon creating the object.
         */
        void init();
    public:
        /*
         * Constructor. Only to be called when we are building the master part list.
         *
         * @model: 3D model of the part.
         * @physics: raw pointer to the physics object. Currently breaks encapsulation :(.
         * @col_shape: collision shape of the part.
         * @mass: dry mass of the part, in kilograms.
         * @baseID: base ID of the object, shared among all copied objects.
         * @asset_manager: pointer to the asset manager interface object.
         */
        BasePart(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, 
                 int baseID, AssetManagerInterface* asset_manager);

        BasePart(const BasePart& part);
        BasePart();
        virtual ~BasePart();

        /*
         * Adds an attachment point to the object. To be called when the part is being initialized
         * in the master part list.
         *
         * @point: local origin of the attachment.
         * @orientation: orientation of the attachment point.
         */
        void addAttachmentPoint(const math::vec3& point, const math::vec3& orientation);

        /*
         * Sets the parent attachment point of the part. This point is where this part will attach
         * itself on other part's attachment points.
         *
         * @point: local origin of the attachment.
         * @orientation: orientation of the attachment point.
         */
        void setParentAttachmentPoint(const math::vec3& point, const math::vec3& orientation);

        /*
         * Sets the free attachment point of the part. This attachment point is used to radially
         * attach the part to other parts.
         * @point: local origin of the attachment.
         * @orientation: orientation of the attachment point.
         */
        void setFreeAttachmentPoint(const math::vec3& point, const math::vec3& orientation);

        /*
         * Sets the parent constraint. This object takes ownership of the constraint and adds it to
         * the dynamics world. This method is not thread-safe.
         *
         * @constraint_uptr: unique pointer to the constraint object.
         */
        void setParentConstraint(std::unique_ptr<btTypedConstraint>&& constraint_uptr);

        /*
         * Removes the Bullet constraint between this part and its parent. This method is not 
         * thread safe.
         */
        void removeParentConstraint();

        /*
         * Sets the pointer to the parent (the part it's attached to) of this part.
         *
         * @parent: the parent of this part.
         */
        void setParent(BasePart* parent);

        /*
         * Adds a child (and its subtree) to this part. This part will (semantically) take 
         * ownership of the pointer. This method is thread safe.
         *
         * @child: rvalue reference to the shared ptr of the child to be attached.
         */
        bool addChild(std::shared_ptr<BasePart>&& child);

        /*
         * Removes a child (and its subtree) from this part. We need to pass the pointer of the
         * child, but maybe it could be the unique ID. This method is not thread safe.
         *
         * @child: raw pointer of the child we want to remove.
         */
        std::shared_ptr<BasePart> removeChild(BasePart* child);

        /*
         * Recursively updates the motion state of the subtree, useful to move trees around. Each
         * child adds its new motion state to the command buffer, the function is called 
         * recursively through all the subtree. The method is a bit convoluted, maybe it could be
         * rewritten at some point. This method is thread safe.
         *
         * @disp: displacement, should be new_position - old_position.
         * @root_origin: new origin of the root of this subtree, when this method is called it
         * should be the new origin of the part we are using to call this method.
         * @rotation: new rotation of the subtree.
         */
        void updateSubTreeMotionState(const btVector3& disp, const btVector3& root_origin,
                                      const btQuaternion& rotation);

        /*
         * When this part and its subtree are attached to a Vessel, m_vessel is be updated to the
         * value of the pointer to that Vessel. This method recursively updates this through all
         * the subtree. If the subtree doesn't belong to any Vessel (only in the editor!), this
         * value should be nullptr.
         *
         * @vessel: raw pointer to the Vessel this subtree has been attached to.
         */
        void updateSubTreeVessel(Vessel* vessel);

        /*
         * Sets m_is_root, should be true if this part is the root of the Vessel it belongs to.
         *
         * @root: true if the current part is the root of the Vessel's tree.
         */
        void setRoot(bool root);

        /*
         * Sets the collision mask of the subtree. Sets m_body->getBroadphaseProxy()->
         * m_collisionFilterMask, see  https://pybullet.org/Bullet/phpBB3/viewtopic.php?t=11865
         * for more information. See core/Physics.hpp for the collision groups (CG_*). This 
         * method should be thread-safe-ish.
         *
         * @mask: collision mask.
         */
        void setCollisionMaskSubTree(short mask);

        /*
         * Returns true if the part has parent attachment point.
         */
        bool hasParentAttPoint() const;

        /*
         * Returns true if the part has free attachment point.
         */
        bool hasFreeAttPoint() const;

        /*
         * Sets the properties of this part, it's a mask. The flags are defined at the top of this
         * file(PART_*).
         *
         * @flags: properties mask.
         */
        void setProperties(long flags);

        /*
         * Adds a resource container to this part.
         * @resource: resource container.
         */
        void addResource(const resource_container& resource);

        /*
         * Removes itself and its subtree form the dynamics world. It also removes the constraints
         * from the dynamics world. Returns the number of objects removed.
         */
        int removeBodiesSubtree();

        /*
         * Clones this part and its subtree. Not thread-safe as it useses Physics::addBody.
         *
         * @current: part where the current part is going to be cloned to. When this method is 
         * called for the first time (first non-recursive call), this should be the part we want
         * the root to be cloned to. The rest of the subtree will be under this part.
         * @is_subtree_root: should be true in the first non-recursive call.
         * @m_radial_clone: used to manage radial clones in the editor, true if it's a radial clone.
         */
        void cloneSubTree(std::shared_ptr<BasePart>& current, bool is_subtree_root,
                          bool m_radial_clone);

        /*
         * Builds the constraints of this part's subtree. Not thread safe, use the buffer
         * m_build_constraint_subtree_buffer to build the subtree, or its AssetManagerInterface
         * equivalent.
         *
         * @parent: pointer to the parent. In the AssetManager call it passes nullptr in the first
         * call, so the tree won't be attached to anything. Pass the parent instead.
         */
        void buildSubTreeConstraints(const BasePart* parent);

        /*
         * Sets the velocity of the subtree. Not thread safe.
         *
         * @velocity: new velocity, in m/s.
         */
        void setSubTreeVelocity(const btVector3& velocity);

        /*
         * Adds the subtree to a render buffer, used by the asset manager so don't pay too much 
         * attention.
         *
         * @buffer: reference to the render buffer.
         * @btv_cam_origin: camera origin.
         */
        void addSubTreeToRenderBuffer(std::vector<object_transform>& buffer,
                                      const btVector3& btv_cam_origin);

        /*
         * Returns all the attachment points of this part.
         */
        const std::vector<struct attachment_point>& getAttachmentPoints() const;

        /*
         * Returns the parent attachment point of this part.
         */
        const struct attachment_point& getParentAttachmentPoint() const;

        /*
         * Returns the free (radial) attachment point of this part.
         */
        const struct attachment_point& getFreeAttachmentPoint() const;

        /*
         * Returns the vessel this part belongs to, nullptr if it doesn't belong to any.
         */
        Vessel* getVessel();

        /*
         * const version of getVessel.
         */
        const Vessel* getVessel() const;

        /*
         * Returns true if this part is the root of a Vessel.
         */
        bool isRoot() const;

        /*
         * Returns the properties flags of this part.
         */
        long getProperties() const;

        /*
         * Returns the parent of this part, nullptr if it doesn't have one.
         */
        BasePart* getParent();

        /*
         * const verstion of getParent.
         */
        const BasePart* getParent() const;

        /*
         * Returns a vector with the childs of this part.
         */
        std::vector<std::shared_ptr<BasePart>>& getChilds();

        /*
         * const version of getChilds.
         */
        const std::vector<std::shared_ptr<BasePart>>& getChilds() const;

        /*
         * Returns the radial clones of this part. To be used only in the editor.
         */
        std::vector<BasePart*>& getClones();

        /*
         * Returns the part this part was radially cloned from, nullptr if none. To be used only
         * in the editor.
         */
        BasePart* getClonedFrom();

        /*
         * Clears the radial clone data from the subtree. To be used only in the editor.
         */
        void clearSubTreeCloneData();

        /*
         * Renders anything that's not the vessel, such as Im-GUI panels.
         */
        virtual void renderOther();

        /*
         * Renders this part. This method should be used in single-threaded applications. Inherited
         * from Object.
         */
        int render();

        /*
         * Renders this part given a transform matrix. Used by the RenderContext using the last 
         * saved transform. Inherited from Object.
         */
        int render(const math::mat4& body_transform);

        /*
         * Function called when the player right clicks this part on the editor. In this 
         * implementation it opens the part editor dialog.
         */
        virtual void onEditorRightMouseButton();

        /*
         * Function called when the player right clicks this part during the simulation. In this 
         * implementation it opens the part simulator dialog.
         */
        virtual void onSimulationRightMouseButton();

        /*
         * Update method of the part, called when the physics thread is stepping. For example, if
         * this part is an engine, we can use this method to request resources from its parent,
         * calculate the force applied by the combustion, update temperature parameters, etc. Use
         * AssetManagerInterface to apply thread-unsafe actions.
         */
        virtual void update();

        /* Performs a generic action (defined above as PART_ACTION_*) such as starting the engine
         * (if this part is an engine), decouple (if this decouples), etc.
         *
         * @action: action to be performed.
         */
        virtual void action(int action);

        /*
         * Requests a resource to this part (should be called by its childs).
         *
         * @requester: who is asking for the resource.
         * @resource_id: id of the resource.
         * @mass: mass being requested, this value will be overwritten with the actual ammount this
         * part is returning.
         */
        void requestResource(const BasePart* requester, std::uint32_t resource_id, float& mass);

        /*  
         * This method allows the caller to obtain a copy of the current object without needing to 
         * know if the object is derived from this class or not (preventing slicing). ALL derived
         * classes HAVE to implement this method, otherwise the copied object won't have access to
         * the members of the derived class (since the compiler doesn't know the real type of the
         * object). Derived classes can override this method by returning a BasePart* pointing to
         * the new object or a pointer of the derived class type. Here's an example of how the
         * overriden method should look like:
         *
         * DerivedPart* DerivedPart::clone() const{
         *     return new DerivedPart(*this);
         * }
         */
        virtual BasePart* clone() const;

        /*
         * Sets new alpha value for the rendering.
         * 
         * @new_alpha: new alpha value
         */
        void setAlpha(float new_alpha);

        /*
         * Used by derived classes to load custom data from parts file. Returns EXIT_SUCCESS on 
         * success, EXIT_FAILURE on error, which should terminate the application.
         *
         * @elem: tinyxml2 xml element with the data of the part that is being loaded.
         */
        int loadCustom(const tinyxml2::XMLElement* elem);

        /*
         * Returns a list with the engines (EngineComponent objects) that this part contains.
         * Derived classes should not reimplement this method, but rather add the engines to the
         * engine list (m_engine_list) in, for example, the constructor. It is important that any
         * regular engine (with the exception of reaction control engines) is added to this list,
         * which will (or is?) used by other components, such as autopilots and attitude control
         * systems.
         */
        const std::vector<EngineComponent*>& getEngineList() const;

        btQuaternion m_user_rotation;
};


#endif
