#ifndef DEBUGDRAWER_HPP
#define DEBUGDRAWER_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


class RenderContext;


/*
 * Class used to debug Bullet, inherits from btIDebugDraw. So far only "drawLine" is implemented.
 * See https://pybullet.org/Bullet/BulletFull/classbtIDebugDraw.html.
 */

class DebugDrawer : public btIDebugDraw{
        btVector3 m_camera_center;

        GLuint m_line_vao, m_vbo_vert, m_color_location, m_debug_alpha_location;

        const RenderContext* m_render_context;
    public:
        DebugDrawer(const RenderContext* render_context);

        /*
         * Sets the current center of the camera. It is needed to translate the points relative to
         * the centered view matrix (Bullet passes doubles).
         */
        void setCameraCenter(const btVector3& camera_center);

        /*
         * Method called before starting drawing, binds shaders and vaos.
         */
        void getReady() const;

        void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
        void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
        void reportErrorWarning(const char *warningString);
        void draw3dText(const btVector3& location, const char *textString);

        /*
         * See DebugDrawModes in https://pybullet.org/Bullet/BulletFull/classbtIDebugDraw.html
         */
        void setDebugMode (int debugMode);
        int getDebugMode () const;

};


#endif
