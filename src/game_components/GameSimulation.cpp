#include <memory>
#include <iostream>

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
#include "../assets/Vessel.hpp"
#include "../assets/BasePart.hpp"
#include "../assets/Object.hpp"
#include "../assets/utils/planet_utils.hpp"
#include "../assets/Kinematic.hpp"
#include "../assets/Model.hpp"
#include "../assets/PlanetarySystem.hpp"
#include "../renderers/SimulationRenderer.hpp"


typedef VesselMap::iterator VesselIterator;


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

    m_renderer.reset(new SimulationRenderer(m_app));
    m_app->getRenderContext()->setRenderer(m_renderer.get(), RENDER_SIMULATION);
    
}


GameSimulation::~GameSimulation(){}


void GameSimulation::update(){
    processInput();
}

void GameSimulation::updateCamera(){
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


void GameSimulation::processKeyboardInput(){
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


void GameSimulation::processInput(){
    double scx, scy;
    m_input->getScroll(scx, scy);
    if((scy) && !m_render_context->imGuiWantCaptureMouse()){
        m_camera->incrementOrbitalCamDistance(-scy * 5.0); // we should check the camera mode
    }

    if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2] & INPUT_MBUTTON_RELEASE &&
        !m_render_context->imGuiWantCaptureMouse() && m_camera->getPrevInputMode() != GLFW_CURSOR_DISABLED){
        onRightMouseButton();
    }
    processKeyboardInput();
}


void GameSimulation::onRightMouseButton(){
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


void GameSimulation::onStateChange(){
    editorToSimulation();
    initLaunchBase();
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
        btVector3 to(-26504446806.42, -38663.47, 144693255800.63);
        to += reference_ellipse_to_xyz(btRadians(0.0), btRadians(0.0), 6371055.0 - vsl->getLowerBound());
        btVector3 disp;
        btTransform transform;

        root->m_body->getMotionState()->getWorldTransform(transform);
        const btVector3& from = transform.getOrigin();

        disp = to - from;

        // move the object to the sea launch base
        vsl->getRoot()->updateSubTreeMotionState(m_asset_manager->m_set_motion_state_buffer,
                                                 disp, from, btQuaternion(0.0, 0.0, -M_PI / 2.0));

        vsl->setVesselVelocity(btVector3(-29786.6, -0.00889649, -5478.81));
        //vsl->setVesselVelocity(btVector3(0.0, 0.0, 0.0));

        m_asset_manager->m_active_vessels.insert({vsl->getId(), vsl});
    }
}


void GameSimulation::initLaunchBase(){
    std::unique_ptr<Model> bigcube(new Model("../data/bigcube.dae", nullptr,
                                              SHADER_PHONG_BLINN_NO_TEXTURE, m_frustum,
                                              m_render_context, math::vec3(0.75, 0.75, 0.75)));
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

