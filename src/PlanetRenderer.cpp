#include <string>
#include <iostream>
#include <thread>
#include <math.h>
#include <mutex>
#include <sstream>

#include <stb/stb_image.h>

#include "PlanetRenderer.hpp"
#include "core/maths_funcs.hpp"
#include "core/RenderContext.hpp"
#include "core/Camera.hpp"
#include "core/WindowHandler.hpp"
#include "core/Input.hpp"
#include "core/Player.hpp"
#include "core/AssetManager.hpp"
#include "core/Physics.hpp"
#include "assets/Model.hpp"
#include "assets/Planet.hpp"
#include "assets/PlanetTree.hpp"
#include "assets/utils/planet_utils.hpp"
#include "GUI/FontAtlas.hpp"



PlanetRenderer::PlanetRenderer() : BaseApp(){
    init();
}


PlanetRenderer::PlanetRenderer(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


void PlanetRenderer::init(){
    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    //m_asset_manager->loadStarSystem();

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);
    m_polygon_mode_lines = false;
}


PlanetRenderer::~PlanetRenderer(){
}

void PlanetRenderer::processInput(){
    if(m_input->pressed_keys[GLFW_KEY_ESCAPE] == INPUT_KEY_DOWN){
        terminate();
    }

    if(m_input->pressed_keys[GLFW_KEY_R] & INPUT_KEY_RELEASE){
        m_render_context->reloadShaders();
        std::cout << "Shaders reloaded" << std::endl;
    }

    if(m_input->pressed_keys[GLFW_KEY_L] & INPUT_KEY_RELEASE){
        if(m_polygon_mode_lines){
            m_polygon_mode_lines = false;
        }
        else{
            m_polygon_mode_lines = true;
        }
    }
}


void PlanetRenderer::terminate(){
    m_window_handler->setWindowShouldClose();
    m_asset_manager->cleanup();
}


void PlanetRenderer::run(){
    Planet planet(m_render_context.get());

    PlanetTree::loadBases(m_frustum.get(), m_render_context.get());

    m_camera->setCameraPosition(dmath::vec3(9300000.0, 0.0, 0.0));
    m_camera->setSpeed(630000.0f);
    m_camera->createProjMat(1.0, 63000000, 67.0, 1.0);

    //glDisable(GL_CULL_FACE);

    m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));
    m_render_context->toggleDebugOverlay();

    while(!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
        m_camera->freeCameraUpdate();

        processInput();

        m_render_context->contextUpdatePlanetRenderer();

        const dmath::vec3& cam_translation = m_camera->getCamPosition();
        planet.render(cam_translation, dmath::identity_mat4());


        if(m_polygon_mode_lines){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else{
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glfwSwapBuffers(m_window_handler->getWindow());
    }

    m_window_handler->terminate();
}

