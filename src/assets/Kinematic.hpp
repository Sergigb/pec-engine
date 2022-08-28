#ifndef KINEMATIC_HPP
#define KINEMATIC_HPP

#include "Object.hpp"


struct iv_array;

class Planet;


/*
 * Class used for kinematics. Since it behaves slightly different from Object, I made a new
 * class. The kinematics can be relative to a planet or absolute.
 */
class Kinematic : public Object{
    //private:
    //    std::unique_ptr<iv_array> m_trimesh;
    private:
        btTransform m_transform;
    public:
        Kinematic(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID);
        Kinematic(const Kinematic& object);
        Kinematic();
        ~Kinematic();

        /*
         * Update method, updates the motion state of the kinematic given its internal origin and 
         * rotation.
         */
        void update();

        /*
         * Update method, updates the motion state of the kinematic given its internal origin and
         * rotation. The kinematic is transformer relative to the given transform.
         *
         * @transform: basis origin of the transform
         */
        void update(const btTransform& transform);

        /*
         * Update method, updates the motion state of the kinematic given its internal origin and
         * rotation. The kinematic is transformer relative given origin and rotation.
         *
         * @transform: basis origin of the transform
         */
        void update(const btVector3& origin, const btQuaternion& rotation);

        /*
         * Sets the transform (relative or absolute) of the kinematic.
         * 
         * @transform: the new transform.
         */
        void setTransform(const btTransform& transform);

        /*
         * Sets the transform (relative or absolute) of the kinematic.
         *
         * @origin: new origin of the kinematic.
         * @rotation: new rotation of the kinematic.
         */
        void setTransform(const btVector3& origin, const btQuaternion& rotation);

        /*
         * Will be gone in the future.
         */
        void renderOther();
        
        /*
         * If the collision object of the Kinematic is an iv_array (see core/Physics.hpp), pass the
         * ownership to this object.
         */
        //void setTrimesh(std::unique_ptr<iv_array>& array);
};


#endif
