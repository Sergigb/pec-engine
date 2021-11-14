#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <memory>
#include <string>
#include <cstdint>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "../core/maths_funcs.hpp"


class Physics;
class Model;


/* Base class for rigid objects.*/
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
        std::unique_ptr<btRigidBody> m_body; // made public for convenience <- this should change lol

        /*
         * Constructor.
         *
         * @model: 3D model of the part.
         * @physics: raw pointer to the physics object. Currently breaks encapsulation :(.
         * @col_shape: collision shape of the part.
         * @mass: dry mass of the part, in kilograms.
         * @baseID: base ID of the object, shared among all copied objects.
         */
        Object(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar dry_mass, std::uint32_t base_id);
        Object();
        Object(const Object& obj);
        virtual ~Object();

        /*
         * Adds body to the dynamics world, so it's not thread safe. This method is subjected to 
         * change to virtual at some point, as I think I'll need to re-implement it in Parts with
         * composed collision objects.
         *
         * @origin: origin of the object.
         * @local_inertia: local inertia vector of the object.
         * @initial_rotation: initial rotation of the object.
         */
        void addBody(const btVector3& origin, const btVector3& local_inertia, const btQuaternion& initial_rotation);

        /*
         * Removes the body (m_body) from the dynamics world. Not thread safe.
         */
        void removeBody();

        /*
         * Returns the single-precision transform matrix of the object.
         */
        math::mat4 getRigidBodyTransformSingle() const;

        /*
         * Returns by reference the single-precision transform matrix of the object.
         */
        void getRigidBodyTransformSingle(math::mat4& body_transform) const;

        /*
         * Returns the double-precision transform matrix of the object.
         *
         * @mat4: should be a 16 (4x4) double array.
         */
        void getRigidBodyTransformDouble(double* mat4) const;

        /*
         * Returns by reference the name of this object.
         *
         * @name: standard c++ string reference, where the name will be stored.
         */
        void getName(std::string& name) const;

        /*
         * Returns by reference the fancy name of this object.
         *
         * @name: standard c++ string reference, where the fancy name will be stored.
         */
        void getFancyName(std::string& name) const;

        /*
         * Returns the unique ID of this object.
         */
        std::uint32_t getUniqueId() const;

        /*
         * Returns the base ID of this object, shared among all objects of the cloned objects.
         */
        std::uint32_t getBaseId() const;

        /*
         * Returns a shared pointer to this object.
         */
        std::shared_ptr<Object> getSharedPtr();

        /*
         * Returns the collision group mask of the rigid body of this object.
         */
        short getCollisionGroup() const;

        /*
         * Returns the collision mask of the rigid body of this object.
         */
        short getCollisionFilters() const;

        /*
         * Returns the total mass of this object.
         */
        double getMass() const;

        /*
         * Returns the dry mass of this object.
         */
        double getDryMass() const;

        /*
         * Applies a central force to this object. Not thread safe.
         *
         * @force: force vector, in newtons.
         */
        void applyCentralForce(const btVector3& force);

        /*
         * Applies torque to the rigid body.
         *
         * @torque: torque vector to be applied, in N*m.
         */
        void applyTorque(const btVector3& torque);

        /*
         * Sets the color of the mesh.
         *
         * @color: new color, in RGB, 0 to 1.
         */
        void setColor(math::vec3 color);

        /*
         * Sets the scale of the mesh.
         *
         * @scale: scale of the mesh.
         */
        void setMeshScale(float scale);

        /*
         * Sets the scale of the mesh.
         *
         * @scale: scale of the mesh, the three elements are the diagonal of the affine transform.
         */
        void setMeshScale(const math::vec3& scale);

        /*
         * Sets a custom transform of the mesh.
         *
         * @transform: single-precision matrix transform.
         */
        void setMeshTransform(const math::mat4& transform);

        /*
         * Sets the motion state of the object. Not thread safe.
         *
         * @origin: new origin of the object.
         * @initial_rotation: initial rotation of the object.
         */
        void setMotionState(const btVector3& origin, const btQuaternion& initial_rotation);

        /*
         * Sets the name of the object.
         *
         * @name: std::string with the name.
         */
        void setName(std::string name);

        /*
         * Sets the fancy name of the object.
         *
         * @name: std::string with the fancy name.
         */
        void setFancyName(std::string name);

        /*
         * Renders this part. This method should be used in single-threaded applications.
         */
        virtual int render();

        /*
         * Renders this part given a transform matrix. Used by the RenderContext using the last 
         * saved transform.
         */
        virtual int render(const math::mat4& body_transform);

        /*
         * Renders anything that's not the vessel, such as Im-GUI panels.
         */
        virtual void renderOther();

        /*
         * Sets the transparency of the mesh.
         *
         * @alpha: alpha value.
         */
        void setAlpha(float alpha);
        
        /*
         * Sets the collision group mask of the rigid body, HAS to be called before adding the
         * object to the dynamics world. See core/Physics.hpp for the collision groups (CG_*).
         *
         * @cg_group: collision group mask.
         */
        void setCollisionGroup(short cg_group);

        /*
         * Sets the collision group filter mask of the rigid body, HAS to be called before adding
         * the object to the dynamics world. See core/Physics.hpp for the collision groups (CG_*).
         *
         * @cg_filters: collision filter mask.
         */
        void setCollisionFilters(short cg_filters);

        virtual void onEditorRightMouseButton();
};


#endif
