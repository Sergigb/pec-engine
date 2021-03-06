#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <memory>
#include <string>
#include <cstdint>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "../core/maths_funcs.hpp"

/* Simple class for rigid objects, contains a rigid body object and its model*/

class Physics;
class Model;


class Object : public std::enable_shared_from_this<Object>{
    protected:
        Model* m_model;
        Physics* m_physics;
        
        std::unique_ptr<btMotionState> m_motion_state;
        math::vec3 m_mesh_color;
        math::mat4 m_mesh_transform;
        bool m_has_transform;
        btScalar m_dry_mass, m_mass;
        btCollisionShape* m_col_shape;
        std::uint32_t m_base_id, m_unique_id;
        std::string m_object_name, m_fancy_name;
        float m_alpha;
        short m_col_group, m_col_filters;
    public:
        std::unique_ptr<btRigidBody> m_body; // made public for convenience

        Object(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar dry_mass, std::uint32_t base_id);
        Object();
        Object(const Object& obj);
        virtual ~Object();

        void addBody(const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation);
        void removeBody(); // this is essentially for objects/kinematics, parts should be disposed accordingly

        math::mat4 getRigidBodyTransformSingle() const;
        void getRigidBodyTransformSingle(math::mat4& body_transform) const;
        void getRigidBodyTransformDouble(double* mat4) const;
        void getName(std::string& name) const;
        void getFancyName(std::string& name) const;
        std::uint32_t getUniqueId() const;
        std::uint32_t getBaseId() const;
        std::shared_ptr<Object> getSharedPtr();
        short getCollisionGroup() const;
        short getCollisionFilters() const;
        double getMass() const;
        double getDryMass() const;

        void applyCentralForce(const btVector3& force);
        void applyTorque(const btVector3& torque);
        void setColor(math::vec3 color);
        void setMeshScale(float scale);
        void setMeshScale(const math::vec3& scale);
        void setMeshTransform(const math::mat4& transform);
        void setMotionState(const btVector3& origin, const btQuaternion& initial_rotation);
        void setName(std::string name);
        void setFancyName(std::string name);
        virtual int render();
        virtual int render(math::mat4 body_transform);
        virtual void renderOther(); // gui, other stuff
        void setAlpha(float alpha);
        // call these before the object is added to the dynamics world
        void setCollisionGroup(short cg_group);
        void setCollisionFilters(short cg_filters);

        virtual void onEditorRightMouseButton();
};


#endif
