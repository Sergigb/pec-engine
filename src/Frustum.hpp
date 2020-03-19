#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP

#include <cmath> 

#include "maths_funcs.hpp"
#include "gl_utils.hpp"

class Frustum{
    private:
        math::vec4 m_planes[6];
    public:
        Frustum();
        ~Frustum();

        void extractPlanes(const math::mat4& proj_mat, const math::mat4& view_mat, bool normalize); //cant make the view mat const?

        bool checkPoint(const math::vec3& p) const;
        bool checkSphere(const math::vec3& p, float radius) const;
        bool checkBox(const math::vec3* pts, const math::mat4& model_mat) const;
};

#endif