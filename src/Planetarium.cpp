#include "Planetarium.hpp"


Planetarium::Planetarium() : BaseApp(){
    init();
}


Planetarium::Planetarium(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


void Planetarium::init(){
    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);
}


Planetarium::~Planetarium(){
}


void Planetarium::run(){

    while (!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
        m_player->update();

        glClearColor(0.2, 0.2, 0.2, 0.2);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSwapBuffers(m_window_handler->getWindow());
    }

    m_window_handler->terminate();
}

