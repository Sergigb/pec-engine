#ifndef KINEMATIC_HPP
#define KINEMATIC_HPP

#include "Object.hpp"



class Kinematic : public Object{
    protected:

    public:
        Kinematic(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID);
        Kinematic(const Kinematic& object);
        Kinematic();
        ~Kinematic();

        void update(); // update position/velocity
};


#endif
