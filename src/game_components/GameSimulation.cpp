#include <memory>
#include <iostream>
#include <algorithm>

#include "GameSimulation.hpp"
#include "../core/log.hpp"
#include "../core/common.hpp"
#include "../core/BaseApp.hpp"
#include "../core/RenderContext.hpp"
#include "../core/Physics.hpp"
#include "../core/Player.hpp"
#include "../core/Camera.hpp"
#include "../core/AssetManager.hpp"
#include "../core/Input.hpp"
#include "../core/WindowHandler.hpp"
#include "../core/Frustum.hpp"
#include "../core/timing.hpp"
#include "../core/Predictor.hpp"
#include "../GUI/DebugOverlay.hpp"
#include "../assets/Vessel.hpp"
#include "../assets/BasePart.hpp"
#include "../assets/Object.hpp"
#include "../assets/utils/planet_utils.hpp"
#include "../assets/Kinematic.hpp"
#include "../assets/Model.hpp"
#include "../assets/PlanetarySystem.hpp"
#include "../assets/Planet.hpp"
#include "../renderers/SimulationRenderer.hpp"
#include "../renderers/PlanetariumRenderer.hpp"
#include "../GUI/planetarium/PlanetariumGUI.hpp"


typedef VesselMap::iterator VesselIterator;


bool comparator(const Planet* a, const Planet* b){
    return a->getOrbitalData().a_0 < b->getOrbitalData().a_0;
}



GameSimulation::GameSimulation(BaseApp* app, const FontAtlas* font_atlas){
    UNUSED(font_atlas);
    m_app = app;
    m_physics = m_app->getPhysics();
    m_player = m_app->getPlayer();
    m_camera = m_app->getCamera();
    m_render_context = m_app->getRenderContext();
    m_asset_manager = m_app->getAssetManager();
    m_input = m_app->getInput();
    m_frustum = m_app->getFrustum();
    m_window_handler = m_app->getWindowHandler();
    m_predictor = m_app->getPredictor();

    m_thread_monitor= m_app->getThreadMonitor();
    m_quit = false;
    m_current_view = VIEW_SIMULATION;
    m_freecam = true;
    m_selected_planet = 0;
    m_selected_planet_idx = 0;

    m_renderer_simulation.reset(new SimulationRenderer(m_app));
    m_app->getRenderContext()->setRenderer(m_renderer_simulation.get(), RENDER_SIMULATION);

    m_gui_planetarium.reset(new PlanetariumGUI(font_atlas, m_app));
    m_render_context->setGUI(m_gui_planetarium.get(), GUI_MODE_PLANETARIUM);
    m_gui_planetarium->setSelectedPlanet(0);

    m_renderer_planetarium.reset(new PlanetariumRenderer(m_app, m_gui_planetarium.get()));
    m_render_context->setRenderer(m_renderer_planetarium.get(), RENDER_PLANETARIUM);

    // ordered planet vector init
    const planet_map& planets = m_asset_manager->m_planetary_system.get()->getPlanets();
    planet_map::const_iterator it;

    for(it=planets.begin();it!=planets.end();it++){
        m_ordered_planets.emplace_back(it->second.get());
    }
    std::sort(m_ordered_planets.begin(), m_ordered_planets.end(), comparator);
}


GameSimulation::~GameSimulation(){}


int GameSimulation::start(){
    double delta_t_ms = (1. / 60.) * 1000000.;
    logic_timing timing;
    timing.delta_t = delta_t_ms;

    m_app->setGUIMode(GUI_MODE_NONE);
    m_app->setRenderState(RENDER_SIMULATION);
    m_player->setBehaviour(PLAYER_BEHAVIOUR_SIMULATION);

    m_physics->pauseSimulation(false);
    m_camera->setCameraPosition(dmath::vec3(9300000.0, 0.0, 0.0));
    m_camera->setSpeed(630000.0f);
    m_camera->createProjMat(1.0, 63000000, 67.0);
    m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));

    while(!m_quit){
        timing.register_tp(TP_LOGIC_START);

        synchPreStep();
        wakePhysics();
        logic();

        m_render_context->getDebugOverlay()->setLogicTimes(timing);

        waitPhysics();
        synchPostStep();

        timing.register_tp(TP_LOGIC_END);
        timing.update();

        if(timing.current_sleep > 0.0){
            std::this_thread::sleep_for(duration(timing.current_sleep));
        }
    }

    return 0;
}


void GameSimulation::synchPostStep(){
    m_asset_manager->updateCoMs();
    if(m_current_view == VIEW_SIMULATION)
        updateCameraSimulation();
    else
        updateCameraPlanetarium();
    m_asset_manager->updateBuffers();
}


void GameSimulation::waitPhysics(){
    std::unique_lock<std::mutex> lck(m_thread_monitor->mtx_end);
    while(!m_thread_monitor->worker_ended){
        m_thread_monitor->cv_end.wait(lck);
    }
    m_thread_monitor->worker_ended = false;
}


void GameSimulation::synchPreStep(){
    m_asset_manager->processCommandBuffers(false);
    m_input->update();
    m_window_handler->update();
    m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
}


void GameSimulation::wakePhysics(){
    std::unique_lock<std::mutex> lck2(m_thread_monitor->mtx_start);
    m_thread_monitor->worker_start = true;
    m_thread_monitor->cv_start.notify_all();
}


void GameSimulation::logic(){
    if(m_current_view == VIEW_SIMULATION)
        processInputSimulation();
    else{
        processKeyboardInputPlanetarium();
        updatePlanetarium();
    }
    processInput();
    m_asset_manager->updateVessels();
}


void GameSimulation::updatePlanetarium(){
    m_app->getPlayer()->setSelectedPlanet(m_selected_planet);
    m_gui_planetarium->setSelectedPlanet(m_selected_planet);
    m_gui_planetarium->setFreecam(m_freecam);
    int action = m_gui_planetarium->update();

    if(action == PLANETARIUM_ACTION_SET_VELOCITY){
        Vessel* vessel = m_app->getPlayer()->getVessel();
        if(vessel){
            const btVector3 velocity = m_gui_planetarium->getCheatVelocity();
            m_app->getAssetManager()->setVesselVelocity(vessel, velocity);
        }
        else{
            std::cerr << "GameSimulation::updatePlanetarium - Can't set vessel velocity because"
                         " Player returned nullptr" << std::endl;
        }
    }
    else if(action == PLANETARIUM_ACTION_SET_POSITION){
        Vessel* vessel = m_app->getPlayer()->getVessel();
        if(vessel){
            const btVector3 origin = m_gui_planetarium->getCheatPosition();
            const btQuaternion rotation = vessel->getRoot()->m_body->getOrientation();
            vessel->setSubTreeMotionState(origin, rotation); // thread safe
        }
        else{
            std::cerr << "GameSimulation::updatePlanetarium - Can't set vessel position because"
                         " Player returned nullptr" << std::endl;
        }
    }
    else if(action == PLANETARIUM_ACTION_SET_ORBIT){
        Vessel* vessel = m_app->getPlayer()->getVessel();
        if(vessel){
            const btQuaternion rotation = vessel->getRoot()->m_body->getOrientation();
            const struct cheat_orbit cheat_orbit_data = 
                m_gui_planetarium->getCheatOrbitParameters();
            double current_time = m_app->getPhysics()->getCurrentTime();
            dmath::vec3 origin, velocity;

            m_predictor->computeObjectPosVel(cheat_orbit_data.cheat_orbit_params,
                                             cheat_orbit_data.body_target, current_time,
                                             cheat_orbit_data.match_frame, origin, velocity);

            vessel->setSubTreeMotionState(btVector3(origin.v[0], origin.v[1], origin.v[2]),
                                          rotation); // thread safe
            m_app->getAssetManager()->setVesselVelocity(vessel, 
                                    btVector3(velocity.v[0], velocity.v[1], velocity.v[2]));
        }
        else{
            std::cerr << "GameSimulation::updatePlanetarium - Can't set vessel position because"
                         "Player returned nullptr" << std::endl;
        }
    }
}


void GameSimulation::processInput(){
    if(m_render_context->imGuiWantCaptureKeyboard()){
        return;
    }

    if(m_input->pressed_keys[GLFW_KEY_ESCAPE] == INPUT_KEY_DOWN){
        m_quit = true;
    }

    if(m_input->pressed_keys[GLFW_KEY_M] == INPUT_KEY_DOWN){
        if(m_player->getBehaviour() & PLAYER_BEHAVIOUR_SIMULATION){
            m_app->setRenderState(RENDER_PLANETARIUM);
            m_app->setGUIMode(GUI_MODE_PLANETARIUM);
            m_player->setBehaviour(PLAYER_BEHAVIOUR_PLANETARIUM);
            m_current_view = VIEW_PLANETARIUM;
        }
        else{
            m_app->setRenderState(RENDER_SIMULATION);
            m_app->setGUIMode(GUI_MODE_NONE);
            m_player->setBehaviour(PLAYER_BEHAVIOUR_SIMULATION);
            m_current_view = VIEW_SIMULATION;
        }
    }

    if(m_input->pressed_keys[GLFW_KEY_F12] == INPUT_KEY_DOWN){
        m_render_context->toggleDebugOverlay();
    }

    if(m_input->pressed_keys[GLFW_KEY_F11] == INPUT_KEY_DOWN){
        m_render_context->toggleDebugDraw();
    }

    if(m_input->pressed_keys[GLFW_KEY_F10] == INPUT_KEY_DOWN){
        m_render_context->reloadShaders();
        m_render_context->setLightPosition(math::vec3(63000000000.0, 0.0, 0.0));
    }
}


void GameSimulation::updateCameraSimulation(){
    const Vessel* vessel = m_player->getVessel();
    if(!vessel){
        m_camera->freeCameraUpdate();
    }
    else if(vessel){ // sanity check for vessel
        const btVector3& com = vessel->getCoM();
        m_camera->setCameraPosition(dmath::vec3(com.getX(), com.getY(), com.getZ()));
        // setCamAxisRotation(); // inherited from Player, should set the up vector of the camera
                                 // and change its inclination. It is not implemented 
        m_camera->orbitalCameraUpdate();
    }
}


void GameSimulation::updateCameraPlanetarium(){
    if(!m_selected_planet || m_freecam){
        m_camera->freeCameraUpdate();
    }
    else if(m_selected_planet && !m_freecam){
        m_camera->setCameraPosition(
            m_asset_manager->m_planetary_system->getPlanets().at(m_selected_planet)->getPosition()
            / PLANETARIUM_SCALE_FACTOR);
        m_camera->setOrbitalInclination(0.0, dmath::vec3(0.0, 0.0, 0.0));
        m_camera->orbitalCameraUpdate();
    }
}


void GameSimulation::switchVessel(){
    VesselIterator it = m_asset_manager->m_active_vessels.find(m_player->getVessel()->getId());

    if(it == m_asset_manager->m_active_vessels.end()){
        std::cerr << "GameSimulation::switchVessel: can't iterate to the next vessel because the "
                     "current vessel's id can't be found in m_active_vessels" << std::endl;
        log("GameSimulation::switchVessel: can't iterate to the next vessel because the " \
            "current vessel's id can't be found in m_active_vessels");
        return;
    }
    it++;
    if(it != m_asset_manager->m_active_vessels.end()){
        m_player->setVessel(it->second.get());
    }
    else{
        it = m_asset_manager->m_active_vessels.begin();
        m_player->setVessel(it->second.get());
    }
}


void GameSimulation::setPlayerTarget(){
    if(m_player->getVessel())
        m_player->unsetVessel();
    else if(m_asset_manager->m_active_vessels.size()){
        VesselIterator it = m_asset_manager->m_active_vessels.begin();
        m_player->setVessel(it->second.get()); // in the future we should pick the closest
    }
}


void GameSimulation::processKeyboardInputSimulation(){
    if(m_render_context->imGuiWantCaptureKeyboard()){
        return;
    }

    // testing
    if(m_input->pressed_keys[GLFW_KEY_F1] == INPUT_KEY_DOWN){
        if(m_player->getVessel()){
            m_player->getVessel()->printVessel();
        }
    }

    if(m_input->pressed_keys[GLFW_KEY_F2] == INPUT_KEY_DOWN){
        if(m_player->getVessel()){
            m_player->getVessel()->printStaging();
        }
    }

    if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)) 
       && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
        setPlayerTarget();
    }

    if(m_input->pressed_keys[GLFW_KEY_TAB] & INPUT_KEY_DOWN && m_player->getVessel()){
        switchVessel();
    }
}


void GameSimulation::processKeyboardInputPlanetarium(){
    if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)) 
        && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){
        m_freecam = !m_freecam;
        if(!m_selected_planet && !m_freecam)
            switchPlanet();
    }

    if(m_input->pressed_keys[GLFW_KEY_TAB] & INPUT_KEY_DOWN && !m_freecam){
        switchPlanet();
    }

    double scx, scy;
    float fade = 0.0; /// this should go somewhere else
    m_input->getScroll(scx, scy);
    if((scy) && !m_render_context->imGuiWantCaptureMouse()){
        double current_distance = m_camera->getOrbitalCamDistance(), increment;

        if(current_distance < 0.15){
            increment = -SIGN(scy) * (0.08 * current_distance);
            fade = 1.0;
            }
        else if(current_distance < 1){ // fade at this interval
            fade = 1.0 - ((current_distance - 0.15) / (1 - 0.15));
            increment = -SIGN(scy) * (0.1 * current_distance);
        }
        else{
            increment = -SIGN(scy) * std::min((std::pow(std::abs(current_distance), 2.0) / 100.0)
                                              + 0.5, 75.0);
            fade = 0.0;
        }
        if(current_distance + increment > 10000.0){
            increment = 10000.0;
            increment = 0.0;
        }
        else if(current_distance + increment < 1e7 / PLANETARIUM_SCALE_FACTOR){
            increment = 0.0;
            current_distance = 1e7 / PLANETARIUM_SCALE_FACTOR;
        }
        m_gui_planetarium->setTargetFade(fade);
        m_renderer_planetarium->setTargetFade(fade);
        m_camera->setOrbitalCamDistance(current_distance + increment); // we should check the camera mode
    }
}


void GameSimulation::processInputSimulation(){
    double scx, scy;
    m_input->getScroll(scx, scy);
    if((scy) && !m_render_context->imGuiWantCaptureMouse()){
        m_camera->incrementOrbitalCamDistance(-scy * 5.0); // we should check the camera mode
    }

    if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2] & INPUT_MBUTTON_RELEASE &&
       !m_render_context->imGuiWantCaptureMouse() && 
       m_camera->getPrevInputMode() != GLFW_CURSOR_DISABLED){
        onRightMouseButton();
    }
    processKeyboardInputSimulation();
}


void GameSimulation::onRightMouseButton(){
    dmath::vec3 ray_start_world, ray_end_world;
    Object* obj;
    BasePart* part;

    m_camera->castRayMousePos(1000.f, ray_start_world, ray_end_world);

    btCollisionWorld::ClosestRayResultCallback ray_callback(btVector3(ray_start_world.v[0], 
                                                ray_start_world.v[1], ray_start_world.v[2]),
                                                            btVector3(ray_end_world.v[0],
                                                ray_end_world.v[1], ray_end_world.v[2]));
    ray_callback.m_collisionFilterGroup = CG_RAY_EDITOR_SELECT;

    obj = m_physics->testRay(ray_callback, 
                             btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                             btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));

    if(obj){
        part = static_cast<BasePart*>(obj);
        part->onSimulationRightMouseButton();
    }
}


void GameSimulation::switchPlanet(){
    if(!m_selected_planet){
        m_selected_planet_idx = 0;  // initialized to 0 but just to be sure
    }
    else{
        m_selected_planet_idx++;
        if(m_selected_planet_idx >= m_ordered_planets.size()){
            m_selected_planet_idx = 0;
        }
    }

    m_selected_planet = m_ordered_planets.at(m_selected_planet_idx)->getId();
}


void GameSimulation::setUpSimulation(){
    editorToSimulation();
    initLaunchBase();  // POSSIBLE SEG FAULT HERE
}


#define ed_subtrees m_asset_manager->m_editor_subtrees
void GameSimulation::editorToSimulation(){
    SubTreeMap::iterator it;
    VesselMap::iterator it2;

    for(it=ed_subtrees.begin(); it != ed_subtrees.end(); it++){
        it->second->removeBodiesSubtree();
    }
    ed_subtrees.clear();

    if(m_asset_manager->m_editor_vessel.get()){
        std::shared_ptr<Vessel> vsl = std::move(m_asset_manager->m_editor_vessel);

        BasePart* root = vsl->getRoot();
        
        // translate to lat/long 0.0-0.0, this function doesn't work well when lat or long != 0, for some reason
        //btVector3 to = reference_ellipse_to_xyz(btRadians(0.0), btRadians(0.0), 6371025.0 - vsl->getLowerBound());
        //btVector3 to(-2.6505e+10, -38663.6, 1.44693e+11);
        btVector3 to(-26504446806.42, -38663.47, 144693255900.63);
        to += reference_ellipse_to_xyz(btRadians(0.0), btRadians(0.0),
                                       6371555.0 - vsl->getLowerBound());
        btVector3 disp;
        btTransform transform;

        root->m_body->getMotionState()->getWorldTransform(transform);
        const btVector3& from = transform.getOrigin();

     /*   btVector3 to(180000000000.0, 0.0, 0.0);
*/
        disp = to - from;

        // move the object to the sea launch base
        vsl->getRoot()->updateSubTreeMotionState(disp, from, btQuaternion(0.0, 0.0, -M_PI / 2.0));

        vsl->setVesselVelocity(btVector3(-29786.6, -0.00889649, -5478.81));
        //vsl->setVesselVelocity(btVector3(0.0, 0.0, 27154.0));
        //vsl->setVesselVelocity(btVector3(0.0, 0.0, 0.0));

        m_asset_manager->m_active_vessels.insert({vsl->getId(), vsl});
    }
}


void GameSimulation::initLaunchBase(){
    std::unique_ptr<Model> bigcube(new Model("../data/bigcube.dae", nullptr,
                                             SHADER_PHONG_BLINN_NO_TEXTURE,
                                             math::vec3(0.75, 0.75, 0.75)));
    std::unique_ptr<btCollisionShape> sphere_shape(new btBoxShape(btVector3(25.0, 25.0, 25.0)));

    btQuaternion quat(0.0, 0.0, 0.0);
    std::shared_ptr<Kinematic> ground = std::make_shared<Kinematic>(bigcube.get(), m_physics, 
                                                                    sphere_shape.get(), 
                                                                    btScalar(0.0), 1);

    btVector3 origin(-26504446806.42, -38663.47, 144693255800.63);
    btVector3 local_origin = reference_ellipse_to_xyz(btRadians(0.0), btRadians(0.0), 6371025.0);

    ground->setCollisionGroup(CG_DEFAULT | CG_KINEMATIC);
    ground->setCollisionFilters(~CG_RAY_EDITOR_RADIAL & ~CG_RAY_EDITOR_SELECT);
    ground->addBody(local_origin + origin, btVector3(0.0, 0.0, 0.0), quat);
    ground->setTransform(local_origin, quat);

    m_asset_manager->m_kinematics.emplace_back(ground);
    m_asset_manager->m_models.push_back(std::move(bigcube));
    m_asset_manager->m_collision_shapes.push_back(std::move(sphere_shape));

    // let's register the kinematic on the earth
    std::hash<std::string> str_hash;
    uint32_t planet_id = str_hash("Earth"); // we want to find the earth :)
    planet_map& planets = m_asset_manager->m_planetary_system->getPlanets();
    Planet* planet = planets.at(planet_id).get();
    planet->registerKinematic(ground.get());
}

