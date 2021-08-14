#ifndef KINEMATIC_HPP
#define KINEMATIC_HPP

#include <memory>

#include "Object.hpp"


struct iv_array;


/*
 * Class used for kinematics. Since it behaves slightly different from Object, I made a new
 * part. In its current state this class doesn't do much, it will have proper functionality when
 * we have terrain or static props in the planets. The constructor works the same as in Object.
 */
class Kinematic : public Object{
    private:
        std::unique_ptr<iv_array> m_trimesh;
    protected:
        btVector3 m_velocity;

    public:
        Kinematic(Model* model, Physics* physics, btCollisionShape* col_shape, btScalar mass, int baseID);
        Kinematic(const Kinematic& object);
        Kinematic();
        ~Kinematic();

        /*
         * Update method, updates the motion state of the kinematic.
         */
        void update();

        /*
         * Will be gone in the future.
         */
        void renderOther();
        
        /*
         * If the collision object of the Kinematic is an iv_array (see core/Physics.hpp), pass the
         * ownership to this object.
         */
        void setTrimesh(std::unique_ptr<iv_array>& array);
};


#endif
