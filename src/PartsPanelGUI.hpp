#ifndef PARTSPANELGUI_HPP
#define PARTSPANELGUI_HPP

#include <map>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "BasePart.hpp"



class PartsPanelGUI{
    private:
        GLuint m_fb, m_tex;
        GLuint m_vao, m_vbo_vert, m_vbo_tex;
        float m_fb_width, m_fb_height; // this is the size of the PANEL fb

        const std::map<int, std::unique_ptr<BasePart>>* m_master_parts_list;
    public:
        PartsPanelGUI(float fb_width, float fb_height);
        ~PartsPanelGUI();

        void setMasterPartList(const std::map<int, std::unique_ptr<BasePart>>* master_parts_list);
        void onFramebufferSizeUpdate(float fb_width, float fb_height);

        void bindTexture();
        void render();
        void update();

        // methods
};

#endif
