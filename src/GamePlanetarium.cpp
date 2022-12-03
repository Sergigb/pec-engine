#include "GamePlanetarium.hpp"
#include "core/BaseApp.hpp"
#include "core/Player.hpp"
#include "core/RenderContext.hpp"
#include "GUI/planetarium/PlanetariumGUI.hpp"


GamePlanetarium::GamePlanetarium(BaseApp* app, const FontAtlas* font_atlas){
    m_app = app;


    m_gui.reset(new PlanetariumGUI(font_atlas, m_app->m_render_context.get(),
                                   m_app->m_camera.get(), m_app->m_physics.get(), 
                                   m_app->m_asset_manager.get()));
    m_app->m_render_context->setGUI(m_gui.get(), GUI_MODE_PLANETARIUM);
}


GamePlanetarium::~GamePlanetarium(){

}


const PlanetariumGUI* GamePlanetarium::getPlanetariumGUI() const{
    return m_gui.get();
}


void GamePlanetarium::update(){
    /*
        INPUT WILL ENTIRELY BE CONTROLLED BY THIS CLASS, THIS IS TEMPORARY
    */
    m_app->m_player->updatePlanetarium();
    m_gui->setSelectedPlanet(m_app->m_player->getPlanetariumSelectedPlanet());
    m_gui->update();
}
