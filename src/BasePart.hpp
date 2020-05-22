#ifndef BASEPART_HPP
#define BASEPART_HPP

#include <vector>
#include <memory>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "Object.hpp"


struct attachment_point{
    btVector3 point; // attachment point translation wrt the center of the object
    btVector3 orientation; // orientation of the attachment point
};


class BasePart : public Object{
    private:
        struct attachment_point m_parent_att_point;
        std::vector<struct attachment_point> m_attachment_points;  // the attachment points of the part
        std::unique_ptr<btTypedConstraint> m_parent_constraint; // the constraint between this part and the parent
    public:
        BasePart(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation, btScalar mass);
        BasePart();
        ~BasePart();

        void addAttachmentPoint(const btVector3& point, const btVector3& orientation);
        void setParentAttachmentPoint(const btVector3& point, const btVector3& orientation);
        void setParentConstraint(btTypedConstraint* constraint);
        void removeParentConstraint();

        const std::vector<struct attachment_point>* getAttachmentPoints() const;
        btTypedConstraint* getParentConstraint() const;
        const struct attachment_point* getParentAttachmentPoint();
};


#endif
