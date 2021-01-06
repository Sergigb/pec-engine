#include <iostream>

#include "App.hpp"
#include "FontAtlas.hpp"
#include "RenderContext.hpp"
#include "AssetManager.hpp"
#include "GameEditor.hpp"
#include "BtWrapper.hpp"
#include "WindowHandler.hpp"


App::App() : BaseApp(){
    init();
}


App::App(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


void App::init(){
    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);

    m_editor.reset(new GameEditor(this, m_def_font_atlas.get()));
}


App::~App(){
}


void App::run(){
    m_bt_wrapper->startSimulation(1.f / 60.f, 0);
    m_render_context->start();

    m_editor->start();
    m_window_handler->setWindowShouldClose();

    m_asset_manager->cleanup();
    m_bt_wrapper->stopSimulation();
    m_render_context->stop();
    m_window_handler->terminate();
}

