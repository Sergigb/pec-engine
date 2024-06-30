#include <cstring>

#include "DebugDrawer.hpp"
#include "RenderContext.hpp"


#pragma GCC diagnostic push  // Temporal, remove when implemented
#pragma GCC diagnostic ignored "-Wunused-parameter"


#define VERTEX_BUF_LEN 6
#define VERTEX_BUF_SIZE VERTEX_BUF_LEN * sizeof(GLfloat)


struct vertexbufferdata{
    GLfloat bufferdata[VERTEX_BUF_LEN];
    void set(const btVector3& p1, const btVector3& p2){
        bufferdata[0] = p1.getX();
        bufferdata[1] = p1.getY();
        bufferdata[2] = p1.getZ();
        bufferdata[3] = p2.getX();
        bufferdata[4] = p2.getY();
        bufferdata[5] = p2.getZ();
    }
    vertexbufferdata(){
        std::memset(bufferdata, 0, VERTEX_BUF_SIZE);
    }
};


struct colorbufferdata{
    GLfloat bufferdata[3];
    colorbufferdata(GLfloat r, GLfloat g, GLfloat b){
        bufferdata[0] = r;
        bufferdata[1] = g;
        bufferdata[2] = b;
    }
};


struct vertexbufferdata vertex_buf;
struct colorbufferdata color_buf(1.0f, 0.0f, 0.0f);


DebugDrawer::DebugDrawer(const RenderContext* render_context) : m_camera_center(btVector3(0.0, 0.0, 0.0)){
    m_render_context = render_context;

    m_color_location = render_context->getUniformLocation(SHADER_DEBUG, "line_color");
    m_debug_alpha_location = m_render_context->getUniformLocation(SHADER_DEBUG, "alpha");

    glGenVertexArrays(1, &m_line_vao);
    glBindVertexArray(m_line_vao);

    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, VERTEX_BUF_SIZE, vertex_buf.bufferdata, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
}


void DebugDrawer::setCameraCenter(const btVector3& camera_center){
    m_camera_center = camera_center;
}


void DebugDrawer::getReady() const{
    m_render_context->useProgram(SHADER_DEBUG);
    m_render_context->bindVao(m_line_vao);
}


void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color){
    /* 
    Things we assume before we call bullet's debug drawing method (call getReady beforehand):
        - shader has been bound
        - vao has been bound
    */
    color_buf = colorbufferdata(color.getX(), color.getY(), color.getZ());
    vertex_buf.set(from - m_camera_center, to - m_camera_center);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferSubData(GL_ARRAY_BUFFER, 0, VERTEX_BUF_SIZE, vertex_buf.bufferdata);
    glUniform3fv(m_color_location, 1, color_buf.bufferdata);
    glUniform1f(m_debug_alpha_location, 1.0);

    glDrawArrays(GL_LINES, 0, VERTEX_BUF_SIZE);
}


void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color){
}


void DebugDrawer::reportErrorWarning(const char *warningString){
}


void DebugDrawer::draw3dText(const btVector3& location, const char *textString){
}


void DebugDrawer::setDebugMode(int debugMode){
}


int DebugDrawer::getDebugMode () const{
    return DBG_DrawWireframe; // just for now
}


#pragma GCC diagnostic pop

