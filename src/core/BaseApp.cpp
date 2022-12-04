#include <iostream>

#include <stb/stb_image.h>

#include "BaseApp.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "WindowHandler.hpp"
#include "Frustum.hpp"
#include "Physics.hpp"
#include "RenderContext.hpp"
#include "AssetManager.hpp"
#include "Player.hpp"
#include "../assets/Model.hpp"
#include "log.hpp"


BaseApp::BaseApp(){
    init(640, 480);
}


BaseApp::BaseApp(int gl_width, int gl_height){
    init(gl_width, gl_height);
}


BaseApp::~BaseApp(){
}


GLuint m_vao, m_vbo_vert, m_vbo_clr, m_vbo_tex, m_loading_texture, m_disp_location;

void BaseApp::displayLoadingScreen(){
    float fb_width, fb_height;
    m_render_context->getDefaultFbSize(fb_width, fb_height);

    glClearColor(0.f, 1.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_disp_location = m_render_context->getUniformLocation(SHADER_GUI, "disp");

    GLfloat vertices[] = {0.0f, 0.0f, fb_width, fb_height, 0.0f, fb_height,
                          0.0f, 0.0f, fb_width, 0.0f, fb_width, fb_height};

    GLfloat color[] = {0.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 0.0f, 0.0f};

    GLfloat tex_coords[] = {0.0f, 0.0f, 1.0f,
                            1.0f, 1.0f, 1.0f,
                            0.0f, 1.0f, 1.0f,
                            0.0f, 0.0f, 1.0f,
                            1.1f, 0.0f, 1.0f,
                            1.0f, 1.0f, 1.0f,};

    glGenVertexArrays(1, &m_vao);
    m_render_context->bindVao(m_vao);

    glGenBuffers(1, &m_vbo_vert);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_vert);
    glVertexAttribPointer(0, 2,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &m_vbo_clr);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_clr);
    glVertexAttribPointer(1, 4,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(1);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), color, GL_STATIC_DRAW);

    glGenBuffers(1, &m_vbo_tex);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo_tex);
    glVertexAttribPointer(2, 3,  GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(2);
    glBufferData(GL_ARRAY_BUFFER, 18 * sizeof(GLfloat), tex_coords, GL_STATIC_DRAW);

    int x, y, n;
    unsigned char* image_data = stbi_load("../data/loading_texture.jpg", &x, &y, &n, 4);
    if(!image_data){
        std::cerr << "BaseApp::displayLoadingScreen: loading texture missing" << std::endl;
        log("BaseApp::displayLoadingScreen: loading texture missing");
    }
    else{
        glGenTextures(1, &m_loading_texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_loading_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    stbi_image_free(image_data);

    m_render_context->useProgram(SHADER_GUI);
    glUniform2f(m_disp_location, 0.0, 0.0);

    m_render_context->bindVao(m_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_loading_texture);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(m_window_handler->getWindow());
}



void BaseApp::init(int gl_width, int gl_height){
    m_input.reset(new Input());
    m_camera.reset(new Camera(dmath::vec3(-0.0f, 50.0f, 50.0f), 67.0f,
                              (float)gl_width / (float)gl_height, 0.1f, 63000000.0f,
                              m_input.get()));
    m_camera->setSpeed(10.f);
    m_window_handler.reset(new WindowHandler(gl_width, gl_height, m_input.get(),
                                             m_camera.get()));
    m_camera->setWindowHandler(m_window_handler.get());
    m_frustum.reset(new Frustum());
    m_render_context.reset(new RenderContext(this));
    m_window_handler->setRenderContext(m_render_context.get());
    displayLoadingScreen();
    m_physics.reset(new Physics(this));
    m_asset_manager.reset(new AssetManager(this));
    m_player.reset(new Player(m_camera.get(), m_asset_manager.get(), m_input.get()));

    std::unique_ptr<Model> att_model(new Model("../data/sphere.dae", nullptr, 
                                     SHADER_PHONG_BLINN_NO_TEXTURE, m_frustum.get(),
                                     m_render_context.get(), math::vec3(1.0, 0.0, 0.0)));
    m_render_context->setAttPointModel(&att_model);

    m_buffers.last_updated = none;

    m_gui_mode = 0;
    m_render_state = 0;
}


void BaseApp::run(){
    while (!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        //m_render_context->render(true);
       // glfwSwapBuffers(m_window_handler->getWindow());
    }
    m_window_handler->terminate();
}

short BaseApp::getGUIMode() const{
    return m_gui_mode;
}


short BaseApp::getRenderState() const{
    return m_render_state;
}


void BaseApp::setRenderState(short state){
    m_render_state = state;
}


void BaseApp::setGUIMode(short mode){
    m_gui_mode = mode;
}


const Input* BaseApp::getInput() const{
    return m_input.get();
}


Input* BaseApp::getInput(){
    return m_input.get();   
}


const Camera* BaseApp::getCamera() const{
    return m_camera.get();
}


Camera* BaseApp::getCamera(){
    return m_camera.get();
}


const WindowHandler* BaseApp::getWindowHandler() const{
    return m_window_handler.get();
}


WindowHandler* BaseApp::getWindowHandler(){
    return m_window_handler.get();
}


const Frustum* BaseApp::getFrustum() const{
    return m_frustum.get();
}


Frustum* BaseApp::getFrustum(){
    return m_frustum.get();
}


const RenderContext* BaseApp::getRenderContext() const{
    return m_render_context.get();
}


RenderContext* BaseApp::getRenderContext(){
    return m_render_context.get();
}


const Physics* BaseApp::getPhysics() const{
    return m_physics.get();
}


Physics* BaseApp::getPhysics(){
    return m_physics.get();
}


const AssetManager* BaseApp::getAssetManager() const{
    return m_asset_manager.get();
}


AssetManager* BaseApp::getAssetManager(){
    return m_asset_manager.get();
}


const Player* BaseApp::getPlayer() const{
    return m_player.get();
}


Player* BaseApp::getPlayer(){
    return m_player.get();
}


struct render_buffers* BaseApp::getRenderBuffers(){
    return &m_buffers;
}


struct thread_monitor* BaseApp::getThreadMonitor(){
    return &m_thread_monitor;
}