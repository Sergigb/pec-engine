#ifndef BASEPART_HPP
#define BASEPART_HPP

#include <vector>
#include <memory>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>
#include <imgui.h>

#include "Object.hpp"
#include "maths_funcs.hpp"


class Vessel;
class AssetManagerInterface;
class Resource;
class Physics;
class Model;


struct attachment_point{
    math::vec3 point; // attachment point translation wrt the center of the object
    math::vec3 orientation; // orientation of the attachment point
};


struct resource_container{
    Resource* resource;
    float mass; // volume can be derived from density
    float max_mass;
};



// basic properties
#define PART_SEPARATES 1
#define PART_IS_CM 2
#define PART_HAS_ENGINE 4


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
        long long int m_properties;

        // editor cloning stuff
        std::vector<BasePart*> m_clones;
        BasePart* m_cloned_from;

        std::vector<struct resource_container> m_resources;

        AssetManagerInterface* m_asset_manager;

        btVector3 m_velocity, m_prev_velocity;  // remove me later

        void decoupleChilds();
        void decoupleSelf();
    public:
        BasePart(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID, AssetManagerInterface* asset_manager);
        BasePart(const BasePart& part);
        BasePart();
        ~BasePart();

        void addAttachmentPoint(const math::vec3& point, const math::vec3& orientation);
        void setParentAttachmentPoint(const math::vec3& point, const math::vec3& orientation);
        void setParentConstraint(std::unique_ptr<btTypedConstraint>& constraint_uptr);
        void setFreeAttachmentPoint(const math::vec3& point, const math::vec3& orientation);
        void removeParentConstraint();
        void setParent(BasePart* parent);
        bool addChild(std::shared_ptr<BasePart>& child);
        std::shared_ptr<BasePart> removeChild(BasePart* child);
        // each child adds its new motion state to the command buffer, the function is called recursively through all the subtree
        void updateSubTreeMotionState(std::vector<struct set_motion_state_msg>& command_buffer, const btVector3& disp, const btVector3& root_origin, const btQuaternion& rotation); // add root origin to rotate
        void updateSubTreeVessel(Vessel* vessel);
        void setRenderIgnoreSubTree();
        void setRoot(bool root);
        void setCollisionMaskSubTree(short mask);
        bool hasParentAttPoint() const;
        bool hasFreeAttPoint() const;
        void setProperties(long long int flags);
        void addResource(const resource_container& resource);
        // removes m_body and the constraint from the dynamics world for all the subtree, should only be called from AssetManager when bullet is not stepping
        int removeBodiesSubtree();
        void cloneSubTree(std::shared_ptr<BasePart>& current, bool is_subtree_root, bool m_radial_clone);
        void buildSubTreeConstraints(const BasePart* parent);
        void setSubTreeVelocity(const btVector3& velocity); // bullet must not be stepping

        const std::vector<struct attachment_point>* getAttachmentPoints() const;
        const btTypedConstraint* getParentConstraint() const;
        const struct attachment_point* getParentAttachmentPoint() const;
        const struct attachment_point* getFreeAttachmentPoint() const;
        const Vessel* getVessel() const;
        Vessel* getVessel();
        bool isRoot() const;

        const BasePart* getParent() const;
        BasePart* getParent();
        std::vector<std::shared_ptr<BasePart>>* getChilds();
        const std::vector<std::shared_ptr<BasePart>>* getChilds() const;
        std::vector<BasePart*> getClones();
        BasePart* getClonedFrom();
        void clearSubTreeCloneData();

        virtual void renderOther();
        virtual void onEditorRightMouseButton();
        virtual void onSimulationRightMouseButton();
        virtual void update();
        virtual int render();
        virtual int render(math::mat4 body_transform);

        void requestResource(const BasePart* requester, std::uint32_t resource_id, float& mass);

        /*  
            This method allows the caller to obtain a copy of the current object without needing to know if the object is derived
            from this class or not (preventing slicing). ALL derived classes HAVE to implement this method, otherwise the copied object 
            won't have access to the members of the derived class (since the compiler doesn't know the real type of the object). 
            Derived classes can override this method by returning a BasePart* pointing to the new object or a pointer of the derived
            class type. Here's an example of how the overriden method should look like:

            DerivedPart* DerivedPart::clone() const{
                return new DerivedPart(*this);
            }

        */
        virtual BasePart* clone() const;

        btQuaternion m_user_rotation;
};


#endif
