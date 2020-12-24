#ifndef KINEMATIC_HPP
#define KINEMATIC_HPP

#include <memory>

#include "Object.hpp"



class Kinematic : public Object{
    private:
        std::unique_ptr<btTriangleIndexVertexArray> m_trimesh;
    protected:

    public:
        Kinematic(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID);
        Kinematic(const Kinematic& object);
        Kinematic();
        ~Kinematic();

        void update(); // update position/velocity
        void renderOther();
        // passes ownership of the triangle mesh to the current object
        void setTrimesh(std::unique_ptr<btTriangleIndexVertexArray>& array);
};


#endif
