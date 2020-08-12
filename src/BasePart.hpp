#ifndef BASEPART_HPP
#define BASEPART_HPP

#include <vector>
#include <memory>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "Object.hpp"
#include "maths_funcs.hpp"
#include "buffers.hpp"


class Vessel;


struct attachment_point{
    math::vec3 point; // attachment point translation wrt the center of the object
    math::vec3 orientation; // orientation of the attachment point
};


class BasePart : public Object{
    private:
        struct attachment_point m_parent_att_point;
        std::vector<struct attachment_point> m_attachment_points;  // the attachment points of the part
        std::unique_ptr<btTypedConstraint> m_parent_constraint; // the constraint between this part and the parent
        BasePart* m_parent;
        std::vector<std::shared_ptr<BasePart>> m_childs;
        Vessel* m_vessel;
        bool m_is_root;
    public:
        BasePart(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID);
        BasePart(const BasePart& part);
        BasePart();
        ~BasePart();

        void addAttachmentPoint(const math::vec3& point, const math::vec3& orientation);
        void setParentAttachmentPoint(const math::vec3& point, const math::vec3& orientation);
        void setParentConstraint(std::unique_ptr<btTypedConstraint>& constraint_uptr);
        void removeParentConstraint();
        void setParent(BasePart* parent);
        bool addChild(std::shared_ptr<BasePart>& child);
        std::shared_ptr<BasePart> removeChild(BasePart* child);
        // each child adds its new motion state to the command buffer, the function is called recursively through all the subtree
        void updateSubTreeMotionState(std::vector<struct set_motion_state_msg>& command_buffer, btVector3 disp, btVector3 root_origin, btQuaternion rotation); // add root origin to rotate
        void updateSubTreeVessel(Vessel* vessel);
        void setRenderIgnoreSubTree();
        void setRoot(bool root);

        const std::vector<struct attachment_point>* getAttachmentPoints() const;
        btTypedConstraint* getParentConstraint() const;
        const struct attachment_point* getParentAttachmentPoint() const;
        Vessel* getVessel() const;
        bool isRoot() const;
        int render();
        int render(math::mat4 body_transform);

        // I don't know if I should make these private (friending Vessel)
        BasePart* getParent() const;
        std::vector<std::shared_ptr<BasePart>>* getChilds();
};


#endif
