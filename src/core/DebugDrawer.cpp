#include <cstring>

#include <imgui.h>

#include "log.hpp"
#include "DebugDrawer.hpp"
#include "RenderContext.hpp"
#include "utils/gl_utils.hpp"
#include "../GUI/FontAtlas.hpp"
#include "../GUI/Text2D.hpp"


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


DebugDrawer::DebugDrawer(const RenderContext* render_context) : 
        m_camera_center(btVector3(0.0, 0.0, 0.0)){
    m_render_context = render_context;

    m_color_location = render_context->getUniformLocation(SHADER_DEBUG, "line_color");
    m_debug_alpha_location = m_render_context->getUniformLocation(SHADER_DEBUG, "alpha");

    m_options = DBG_NoDebug;

    glGenVertexArrays(1, &m_line_vao);
    glBindVertexArray(m_line_vao);

    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferData(GL_ARRAY_BUFFER, VERTEX_BUF_SIZE, vertex_buf.bufferdata, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);

    check_gl_errors(true, "DebugDrawer::DebugDrawer");
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

    check_gl_errors(true, "DebugDrawer::drawLine");
}


void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, 
                                   btScalar distance, int lifeTime, const btVector3& color){
    btVector3 end_point = PointOnB + normalOnB * distance * 30;
    color_buf = colorbufferdata(color.getX(), color.getY(), color.getZ());
    vertex_buf.set(end_point - m_camera_center, PointOnB - m_camera_center);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glBufferSubData(GL_ARRAY_BUFFER, 0, VERTEX_BUF_SIZE, vertex_buf.bufferdata);
    glUniform3fv(m_color_location, 1, color_buf.bufferdata);
    glUniform1f(m_debug_alpha_location, 1.0);

    glDrawArrays(GL_LINES, 0, VERTEX_BUF_SIZE);

    check_gl_errors(true, "DebugDrawer::drawContactPoint");
}


void DebugDrawer::reportErrorWarning(const char *warningString){
    float c[4] = {1.f, 0.f, 0.f, 1.f};
    std::wstring wstr(warningString, warningString + strlen(warningString));
    m_text->addString(wstr.c_str(), 400., 400., 1.0, STRING_DRAW_ABSOLUTE_BL,
                      STRING_ALIGN_RIGHT, c);
    m_text->render();

    std::cerr << warningString << std::endl;
    log(warningString);
}


void DebugDrawer::draw3dText(const btVector3& location, const char *textString){
    std::cerr << "missing projection on screen" << std::endl;
    float c[4] = {1.f, 0.f, 0.f, 1.f};
    std::wstring wstr(textString, textString + strlen(textString));
    m_text->addString(wstr.c_str(), 400., 400., 1.0, STRING_DRAW_ABSOLUTE_BL,
                      STRING_ALIGN_RIGHT, c);
    m_text->render();
}


int DebugDrawer::getDebugMode() const{
    return m_options;
}


void DebugDrawer::setDebugMode(int mode){

}


void DebugDrawer::renderImGUI(){
    ImGui::Begin("Debug drawer settings", NULL);
    bool check;

    check = m_options & DBG_DrawWireframe;
    ImGui::Checkbox("DBG_DrawWireframe", &check);
    if(check)
        m_options |= DBG_DrawWireframe;
    else
        m_options &= ~DBG_DrawWireframe;

    check = m_options & DBG_DrawAabb;
    ImGui::Checkbox("DBG_DrawAabb", &check);
    if(check)
        m_options |= DBG_DrawAabb;
    else
        m_options &= ~DBG_DrawAabb;

    check = m_options & DBG_DrawFeaturesText;
    ImGui::Checkbox("DBG_DrawFeaturesText", &check);
    if(check)
        m_options |= DBG_DrawFeaturesText;
    else
        m_options &= ~DBG_DrawFeaturesText;

    check = m_options & DBG_DrawContactPoints;
    ImGui::Checkbox("DBG_DrawContactPoints", &check);
    if(check)
        m_options |= DBG_DrawContactPoints;
    else
        m_options &= ~DBG_DrawContactPoints;

    check = m_options & DBG_NoDeactivation;
    ImGui::Checkbox("DBG_NoDeactivation", &check);
    if(check)
        m_options |= DBG_NoDeactivation;
    else
        m_options &= ~DBG_NoDeactivation;

    check = m_options & DBG_NoHelpText;
    ImGui::Checkbox("DBG_NoHelpText", &check);
    if(check)
        m_options |= DBG_NoHelpText;
    else
        m_options &= ~DBG_NoHelpText;

    check = m_options & DBG_DrawText;
    ImGui::Checkbox("DBG_DrawText", &check);
    if(check)
        m_options |= DBG_DrawText;
    else
        m_options &= ~DBG_DrawText;

    check = m_options & DBG_ProfileTimings;
    ImGui::Checkbox("DBG_ProfileTimings", &check);
    if(check)
        m_options |= DBG_ProfileTimings;
    else
        m_options &= ~DBG_ProfileTimings;

    check = m_options & DBG_EnableSatComparison;
    ImGui::Checkbox("DBG_EnableSatComparison", &check);
    if(check)
        m_options |= DBG_EnableSatComparison;
    else
        m_options &= ~DBG_EnableSatComparison;

    check = m_options & DBG_DisableBulletLCP;
    ImGui::Checkbox("DBG_DisableBulletLCP", &check);
    if(check)
        m_options |= DBG_DisableBulletLCP;
    else
        m_options &= ~DBG_DisableBulletLCP;

    check = m_options & DBG_EnableCCD;
    ImGui::Checkbox("DBG_EnableCCD", &check);
    if(check)
        m_options |= DBG_EnableCCD;
    else
        m_options &= ~DBG_EnableCCD;

    check = m_options & DBG_DrawConstraints;
    ImGui::Checkbox("DBG_DrawConstraints", &check);
    if(check)
        m_options |= DBG_DrawConstraints;
    else
        m_options &= ~DBG_DrawConstraints;

    check = m_options & DBG_DrawConstraintLimits;
    ImGui::Checkbox("DBG_DrawConstraintLimits", &check);
    if(check)
        m_options |= DBG_DrawConstraintLimits;
    else
        m_options &= ~DBG_DrawConstraintLimits;

    check = m_options & DBG_FastWireframe;
    ImGui::Checkbox("DBG_FastWireframe", &check);
    if(check)
        m_options |= DBG_FastWireframe;
    else
        m_options &= ~DBG_FastWireframe;

    check = m_options & DBG_DrawNormals;
    ImGui::Checkbox("DBG_DrawNormals", &check);
    if(check)
        m_options |= DBG_DrawNormals;
    else
        m_options &= ~DBG_DrawNormals;

    check = m_options & DBG_DrawFrames;
    ImGui::Checkbox("DBG_DrawFrames", &check);
    if(check)
        m_options |= DBG_DrawFrames;
    else
        m_options &= ~DBG_DrawFrames;

    if (ImGui::Button("Toggle all")){
        std::cout << m_options << std::endl;
        if(m_options != DBG_NoDebug)
            m_options = DBG_NoDebug;
        else
            m_options = ~0;
    }

    ImGui::End();
}


void DebugDrawer::setAtlas(const FontAtlas* atlas, int fb_width, int fb_height){
    m_text.reset(new Text2D(fb_width, fb_height, atlas, m_render_context));
}


#pragma GCC diagnostic pop

