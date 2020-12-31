#include "DebugDrawer.hpp"


#pragma GCC diagnostic push  // Temporal, remove when implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"

DebugDrawer::DebugDrawer(){
    
}


void DebugDrawer::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color){

}


void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color){

}


void DebugDrawer::reportErrorWarning(const char *warningString){

}


void DebugDrawer::draw3dText(const btVector3& location, const char *textString){

}


void DebugDrawer::setDebugMode (int debugMode){

}


int DebugDrawer::getDebugMode () const{
    return 0;
}


#pragma GCC diagnostic pop

