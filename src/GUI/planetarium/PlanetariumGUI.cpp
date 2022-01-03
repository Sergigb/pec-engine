#include "PlanetariumGUI.hpp"
#include "../FontAtlas.hpp"
#include "../Text2D.hpp"
#include "../../core/RenderContext.hpp"
#include "../../core/log.hpp"



PlanetariumGUI::PlanetariumGUI(const FontAtlas* atlas, const RenderContext* render_context){
    m_font_atlas = atlas;
    m_render_context = render_context;
    m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
    m_fb_update = true;

    m_main_text.reset(new Text2D(m_fb_width, m_fb_height, color{0.85, 0.85, 0.85},
                      m_font_atlas, render_context));

    m_main_text->addString(L"This is a test string, success!", m_fb_width / 2, m_fb_height / 2,
                            1, STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);

}


PlanetariumGUI::~PlanetariumGUI(){}


void PlanetariumGUI::onFramebufferSizeUpdate(){
    m_fb_update = true;
}


void PlanetariumGUI::render(){
    if(m_fb_update){
        m_render_context->getDefaultFbSize(m_fb_width, m_fb_height);
        m_main_text->onFramebufferSizeUpdate(m_fb_width, m_fb_height);

        std::cout << m_fb_width / 2 <<  " - " << m_fb_height / 2 << std::endl;
        m_main_text->clearStrings();
        m_main_text->addString(L"This is a test string, success!", m_fb_width / 2, m_fb_height / 2,
                               1, STRING_DRAW_ABSOLUTE_BL, STRING_ALIGN_CENTER_XY);
        m_fb_update = false;
    }

    m_main_text->render();
}


int PlanetariumGUI::update(){
    return PLANETARIUM_ACTION_NONE;
}
