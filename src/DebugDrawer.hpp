#ifndef DEBUGDRAWER_HPP
#define DEBUGDRAWER_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


class RenderContext;


class DebugDrawer : public btIDebugDraw{
        btVector3 m_camera_center;

        GLuint m_line_vao, m_vbo_vert, m_color_location;

        const RenderContext* m_render_context;
    public:
        DebugDrawer(const RenderContext* render_context);

        void setCameraCenter(const btVector3& camera_center);
        void getReady() const;

        void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
        void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
        void reportErrorWarning(const char *warningString);
        void draw3dText(const btVector3& location, const char *textString);
        void setDebugMode (int debugMode);
        int getDebugMode () const;

};


#endif
