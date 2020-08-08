#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <algorithm>
#include <memory>
#include <string>
#include <cstdint>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "Model.hpp"
#include "maths_funcs.hpp"
#include "BtWrapper.hpp"
#include "id_manager.hpp"

/* Simple class for rigid objects, contains a rigid body object and its model*/

class Object : std::enable_shared_from_this<Object>{
    protected:
        Model* m_model;
        BtWrapper* m_bt_wrapper;
        
        std::unique_ptr<btMotionState> m_motion_state;
        math::vec3 m_mesh_color;
        math::mat4 m_mesh_transform;
        bool m_has_transform;
        btScalar m_mass;
        btCollisionShape* m_col_shape;
        std::uint32_t m_base_id, m_unique_id;
        std::string m_object_name, m_fancy_name;
    public:
        std::unique_ptr<btRigidBody> m_body; // made public for convenience

        Object(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, std::uint32_t base_id);
        Object();
        Object(const Object& obj);
        virtual ~Object();

        void addBody(const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation);

        math::mat4 getRigidBodyTransformSingle() const;
        void getRigidBodyTransformSingle(math::mat4& body_transform) const;
        void getRigidBodyTransformDouble(double* mat4) const;
        void getName(std::string& name) const;
        void getFancyName(std::string& name) const;
        std::uint32_t getUniqueId() const;
        std::uint32_t getBaseId() const;

        void applyCentralForce(const btVector3& force);
        void applyTorque(const btVector3& torque);
        void setColor(math::vec3 color);
        void setMeshScale(float scale);
        void setMeshScale(const math::vec3& scale);
        void setMeshTransform(const math::mat4& transform);
        void setMotionState(const btVector3& origin, const btQuaternion& initial_rotation);
        void setName(std::string name);
        void setFancyName(std::string name);
        void activate(bool activate);
        int render();
        int render(math::mat4 body_transform);
};


#endif
