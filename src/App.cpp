#include <iostream>

#include "App.hpp"
#include "GUI/FontAtlas.hpp"
#include "core/RenderContext.hpp"
#include "core/AssetManager.hpp"
#include "GameEditor.hpp"
#include "core/Physics.hpp"
#include "core/WindowHandler.hpp"

// delete later most likely when the simulation is moved somewhere else
#include <unordered_map>

#include "core/Input.hpp"
#include "core/Camera.hpp"
#include "core/Frustum.hpp"
#include "core/Player.hpp"
#include "core/log.hpp"
#include "assets/BasePart.hpp"
#include "assets/Vessel.hpp"
#include "assets/Model.hpp"
#include "assets/Kinematic.hpp"
#include "assets/utils/planet_utils.hpp"


App::App() : BaseApp(){
    init();
}


App::App(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


void App::init(){
    m_quit = false;

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);

    m_render_context->setDefaultFontAtlas(m_def_font_atlas.get());
    m_editor.reset(new GameEditor(this, m_def_font_atlas.get()));
}


App::~App(){
}


void App::run(){
    m_physics->startSimulation(1. / 60., 0);
    m_render_context->start();

    m_editor->start();

    log("App::run: game state changed from editor to simulation");

    editorToSimulation();
    initLaunchBase();

    /* Simple version of the simulator */

    std::chrono::steady_clock::time_point loop_start_load;
    std::chrono::steady_clock::time_point previous_loop_start_load = std::chrono::steady_clock::now();;
    std::chrono::steady_clock::time_point loop_end_load;
    double delta_t = (1. / 60.) * 1000000., accumulated_load = 0.0, accumulated_sleep = 0.0, average_load = 0.0, average_sleep = 0.0;
    int ticks_since_last_update = 0;

    m_physics->pauseSimulation(false);
    m_gui_mode = GUI_MODE_NONE;
    m_render_state = RENDER_UNIVERSE;
    m_player->setBehaviour(PLAYER_BEHAVIOUR_SIMULATION);
    m_camera->setCameraPosition(dmath::vec3(9300000.0, 0.0, 0.0));
    m_camera->setSpeed(630000.0f);
    m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));

    while(!m_quit){
        loop_start_load = std::chrono::steady_clock::now();

        m_asset_manager->processCommandBuffers(false);

        m_input->update();
        m_window_handler->update();
        m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);

        {  //wake up physics thread
            std::unique_lock<std::mutex> lck2(m_thread_monitor.mtx_start);
            m_thread_monitor.worker_start = true;
            m_thread_monitor.cv_start.notify_all();
        }

        m_asset_manager->updateVessels();
        logic();

        m_render_context->setDebugOverlayTimes(m_physics->getAverageLoadTime(), average_load, average_sleep);
        
        m_elapsed_time += loop_start_load - previous_loop_start_load;
        previous_loop_start_load = loop_start_load;
        
        if(ticks_since_last_update == 60){
            ticks_since_last_update = 0;
            average_load = accumulated_load / 60000.0;
            average_sleep = accumulated_sleep / 60000.0;
            accumulated_load = 0;
            accumulated_sleep = 0;
            /*std::cout << std::setfill('0') << std::setw(2) << int(m_elapsed_time.count() / 1e12) / 60*60 << ":" 
                      << std::setfill('0') << std::setw(2) << (int(m_elapsed_time.count() / 1e6) / 60) % 60 << ":" 
                      << std::setfill('0') << std::setw(2) << int(m_elapsed_time.count() / 1e6) % 60 << std::endl;*/
        }
        ticks_since_last_update++;

        { // wait for physics thread
            std::unique_lock<std::mutex> lck(m_thread_monitor.mtx_end);
            while(!m_thread_monitor.worker_ended){
                m_thread_monitor.cv_end.wait(lck);
            }
            m_thread_monitor.worker_ended = false;
        }

        m_asset_manager->updateCoMs();
        m_asset_manager->updateKinematics();
        m_player->update();
        m_asset_manager->updateBuffers();
        
        // load ends here

    //dmath::vec3 pos = m_camera->getCamPosition();
    //std::cout << pos.v[0] << " " << pos.v[1] << " " << pos.v[2] << std::endl;


        loop_end_load = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::micro> load_time = loop_end_load - loop_start_load;
        accumulated_load += load_time.count();
        accumulated_sleep += delta_t - load_time.count();

        if(load_time.count() < delta_t){
            std::chrono::duration<double, std::micro> delta_ms(delta_t - load_time.count());
            std::this_thread::sleep_for(delta_ms);
        }
    }

    m_window_handler->setWindowShouldClose();

    m_asset_manager->cleanup();
    m_physics->stopSimulation();
    m_render_context->stop();
    m_window_handler->terminate();
}


void App::logic(){
    processInput();
}


void App::processInput(){
    double scx, scy;
    m_input->getScroll(scx, scy);
    if((scy) && !m_render_context->imGuiWantCaptureMouse()){
        m_camera->incrementOrbitalCamDistance(-scy * 5.0); // we should check the camera mode
    }

    if(m_input->pressed_keys[GLFW_KEY_F12] == INPUT_KEY_DOWN && !m_render_context->imGuiWantCaptureKeyboard()){
        m_render_context->toggleDebugOverlay();
    }

    if(m_input->pressed_keys[GLFW_KEY_F11] == INPUT_KEY_DOWN && !m_render_context->imGuiWantCaptureKeyboard()){
        m_render_context->toggleDebugDraw();
    }

    if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2] & INPUT_MBUTTON_RELEASE &&
        !m_render_context->imGuiWantCaptureMouse() && m_camera->getPrevInputMode() != GLFW_CURSOR_DISABLED){
        onRightMouseButton();
    }

    if(m_input->pressed_keys[GLFW_KEY_F10] == INPUT_KEY_DOWN && !m_render_context->imGuiWantCaptureKeyboard()){
        m_render_context->reloadShaders();
        m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));
    }

    if(m_input->pressed_keys[GLFW_KEY_ESCAPE] == INPUT_KEY_DOWN && !m_render_context->imGuiWantCaptureKeyboard()){
        m_quit = true;
    }

    // testing
    if(m_input->pressed_keys[GLFW_KEY_F1] == INPUT_KEY_DOWN && !m_render_context->imGuiWantCaptureKeyboard()){
        if(m_player->getVessel()){
            m_player->getVessel()->printVessel();
        }
    }

    if(m_input->pressed_keys[GLFW_KEY_F2] == INPUT_KEY_DOWN && !m_render_context->imGuiWantCaptureKeyboard()){
        if(m_player->getVessel()){
            m_player->getVessel()->printStaging();
        }
    }
}


void App::onRightMouseButton(){
    dmath::vec3 ray_start_world, ray_end_world;
    Object* obj;
    BasePart* part;

    m_camera->castRayMousePos(1000.f, ray_start_world, ray_end_world);

    btCollisionWorld::ClosestRayResultCallback ray_callback(btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                                            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
    ray_callback.m_collisionFilterGroup = CG_RAY_EDITOR_SELECT;

    obj = m_physics->testRay(ray_callback, 
                                btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));

    if(obj){
        part = static_cast<BasePart*>(obj);
        part->onSimulationRightMouseButton();
    }
}


#define ed_subtrees m_asset_manager->m_editor_subtrees
void App::editorToSimulation(){
    std::unordered_map<std::uint32_t, std::shared_ptr<BasePart>>::iterator it;
    std::unordered_map<std::uint32_t, std::shared_ptr<Vessel>>::iterator it2;

    for(it=ed_subtrees.begin(); it != ed_subtrees.end(); it++){
        it->second->removeBodiesSubtree();
    }
    ed_subtrees.clear();

    if(m_asset_manager->m_editor_vessel.get()){
        std::shared_ptr<Vessel> vsl = std::move(m_asset_manager->m_editor_vessel);

        BasePart* root = vsl->getRoot();
        
        // translate to lat/long 0.0-0.0, this function doesn't work well when lat or long != 0, for some reason
        btVector3 to = reference_ellipse_to_xyz(btRadians(0.0), btRadians(0.0), 6371025.0 - vsl->getLowerBound());
        btVector3 disp;
        btTransform transform;

        root->m_body->getMotionState()->getWorldTransform(transform);
        const btVector3& from = transform.getOrigin();

        disp = to - from;

        // move the object to the sea launch base
        vsl->getRoot()->updateSubTreeMotionState(m_asset_manager->m_set_motion_state_buffer,
                                                 disp, from, btQuaternion(0.0, 0.0, -M_PI / 2.0));

        //vsl->setVesselVelocity(btVector3(0.0, 7788.54, 0.0));
        vsl->setVesselVelocity(btVector3(0.0, 0.0, 0.0));

        m_asset_manager->m_active_vessels.insert({vsl->getId(), vsl});
    }
}


void App::initLaunchBase(){
    std::unique_ptr<Model> bigcube(new Model("../data/bigcube.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE,
                                   m_frustum.get(), m_render_context.get(), math::vec3(0.75, 0.75, 0.75)));
    std::unique_ptr<btCollisionShape> sphere_shape(new btBoxShape(btVector3(25.0, 25.0, 25.0)));

    btVector3 origin = reference_ellipse_to_xyz(btRadians(0.0), btRadians(0.0), 6371000.0);
    btQuaternion quat(0.0, 0.0, 0.0);
    std::shared_ptr<Kinematic> ground = std::make_shared<Kinematic>(bigcube.get(), m_physics.get(), 
                                                                    sphere_shape.get(), btScalar(0.0), 1);
    ground->setCollisionGroup(CG_DEFAULT | CG_KINEMATIC);
    ground->setCollisionFilters(~CG_RAY_EDITOR_RADIAL & ~CG_RAY_EDITOR_SELECT);
    ground->addBody(origin, btVector3(0.0, 0.0, 0.0), quat);

    m_asset_manager->m_kinematics.emplace_back(ground);
    m_asset_manager->m_models.push_back(std::move(bigcube));
    m_asset_manager->m_collision_shapes.push_back(std::move(sphere_shape));
}