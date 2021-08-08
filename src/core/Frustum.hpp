#ifndef FRUSTUM_HPP
#define FRUSTUM_HPP

#include "maths_funcs.hpp"


/*
 * Class is used to perform frustum culling.
 */

class Frustum{
    private:
        math::vec4 m_planes[6];
    public:
        Frustum();
        ~Frustum();

        /*
         * Updates frustum planes given the projection and view matrices.
         *
         * @proj_mat: projection matrix.
         * @view_mat: view matrix, should be the centered view matrix.
         * @normalize: if true the planes are normalized.
         */
        void extractPlanes(const math::mat4& proj_mat, const math::mat4& view_mat, bool normalize);

        /*
         * Checks if a point is inside the frustum.
         *
         * @p: the point.
         */
        bool checkPoint(const math::vec3& p) const;

        /*
         * Checks if a sphere is inside of (or clipping) the frustum. Doesn't seem to work with 
         * extreme fields of view.
         *
         * @p: center of the sphere.
         * @radius: radius of the sphere.
         */
        bool checkSphere(const math::vec3& p, float radius) const;

        /*
         * Checks if a box is inside of (or clipping) the frustum.
         *
         * @pts: corners of the box centered at the origin, should be a vector of size 8.
         * @model_mat: transform matrix of the box, pts are expected to be centered at the
         * origin, if this is not the case this matrix can be the identity.
         */
        bool checkBox(const math::vec3* pts, const math::mat4& model_mat) const;
};

#endif