#include "Frustum.hpp"


Frustum::Frustum(){

}


Frustum::~Frustum(){

}


void Frustum::extractPlanes(const math::mat4& proj_mat, const math::mat4& view_mat, bool normalize){
    math::mat4 viewproj = view_mat * proj_mat;

    m_planes[0].v[0] = viewproj.m[3] + viewproj.m[0];
    m_planes[0].v[1] = viewproj.m[7] + viewproj.m[4];
    m_planes[0].v[2] = viewproj.m[11] + viewproj.m[8];
    m_planes[0].v[3] = viewproj.m[15] + viewproj.m[12];

    m_planes[1].v[0] = viewproj.m[3] - viewproj.m[0];
    m_planes[1].v[1] = viewproj.m[7] - viewproj.m[4];
    m_planes[1].v[2] = viewproj.m[11] - viewproj.m[8];
    m_planes[1].v[3] = viewproj.m[15] - viewproj.m[12];

    m_planes[2].v[0] = viewproj.m[3] - viewproj.m[1];
    m_planes[2].v[1] = viewproj.m[7] - viewproj.m[5];
    m_planes[2].v[2] = viewproj.m[11] - viewproj.m[9];
    m_planes[2].v[3] = viewproj.m[15] - viewproj.m[13];

    m_planes[3].v[0] = viewproj.m[3] + viewproj.m[1];
    m_planes[3].v[1] = viewproj.m[7] + viewproj.m[5];
    m_planes[3].v[2] = viewproj.m[11] + viewproj.m[9];
    m_planes[3].v[3] = viewproj.m[15] + viewproj.m[13];

    m_planes[4].v[0] = viewproj.m[3] + viewproj.m[2];
    m_planes[4].v[1] = viewproj.m[7] + viewproj.m[6];
    m_planes[4].v[2] = viewproj.m[11] + viewproj.m[10];
    m_planes[4].v[3] = viewproj.m[15] + viewproj.m[14];

    m_planes[5].v[0] = viewproj.m[3] - viewproj.m[2];
    m_planes[5].v[1] = viewproj.m[7] - viewproj.m[6];
    m_planes[5].v[2] = viewproj.m[11] - viewproj.m[10];
    m_planes[5].v[3] = viewproj.m[15] - viewproj.m[14];

    if(normalize){
        math::normalise(m_planes[0]);
        math::normalise(m_planes[1]);
        math::normalise(m_planes[2]);
        math::normalise(m_planes[3]);
        math::normalise(m_planes[4]);
        math::normalise(m_planes[5]);
    }
}


bool Frustum::checkPoint(const math::vec3 p){
    for(int i=0; i < 6; i++){
        if(math::dot(m_planes[i], p) < 0.0f)
            return false;
    }
    return true;
}


bool Frustum::checkSphere(const math::vec3 p, float radius){
    // doesn't seem to be working right
    for(int i=0; i < 6; i++){
        float d = math::dot(m_planes[i], p);
        if(d < -radius)
            return false;
        else if(fabs(d) < radius)
            return true;
    }
    return true;
}


bool Frustum::checkBox(const math::vec3* pts){
    // not tested yet
    int in, out;
    for(int i=0; i < 6; i++){
        in = 0; out = 0;
        for(int j=0; j < 8 && (in==0 || out==0); j++){
            if(math::dot(m_planes[i], pts[j]) < 0.0f)
                out++;
            else
                in++;
        }
        if(!in)
            return false;
    }
    return true;
}
