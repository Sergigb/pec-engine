#ifndef KINEMATIC_HPP
#define KINEMATIC_HPP

#include <memory>

#include "Object.hpp"


struct iv_array;


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

        void update(); // update position/velocity
        void renderOther();
        // passes ownership of the triangle mesh to the current object
        void setTrimesh(std::unique_ptr<iv_array>& array);
};


#endif
