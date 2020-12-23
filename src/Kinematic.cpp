#include "Kinematic.hpp"
#include "Object.hpp"
#include "AssetManagerInterface.hpp"


Kinematic::Kinematic(Model* model, BtWrapper* bt_wrapper, btCollisionShape* col_shape, btScalar mass, int baseID) : 
    Object(model, bt_wrapper, col_shape, mass, baseID){

}


Kinematic::Kinematic(const Kinematic& object) : Object(object){

}


Kinematic::Kinematic(){

}


Kinematic::~Kinematic(){

}


void Kinematic::update(){
    
}

