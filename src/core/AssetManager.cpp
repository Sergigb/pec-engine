#include <iostream>
#include <functional>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>

#include "AssetManager.hpp"
#include "RenderContext.hpp"
#include "Frustum.hpp"
#include "Physics.hpp"
#include "BaseApp.hpp"
#include "Camera.hpp"
#include "log.hpp"
#include "../assets/Resource.hpp"
#include "../assets/Model.hpp"
#include "../assets/Object.hpp"
#include "../assets/BasePart.hpp"
#include "../assets/Kinematic.hpp"
#include "../assets/Vessel.hpp"

typedef std::unordered_map<std::uint32_t, std::shared_ptr<BasePart>>::iterator SubTreeIterator;
typedef std::unordered_map<std::uint32_t, std::shared_ptr<Vessel>>::iterator VesselIterator;
typedef std::unordered_map<std::uint32_t, std::unique_ptr<Resource>>::iterator ResourceIterator;


AssetManager::AssetManager(BaseApp* app){
    m_render_context = app->m_render_context.get();
    m_frustum = app->m_frustum.get();
    m_physics = app->m_physics.get();
    m_buffers = &app->m_buffers;
    m_camera = app->m_camera.get();
    m_app = app;

    m_asset_manager_interface = AssetManagerInterface(this);

    initResources();
    objectsInit();
    load_parts(*this);
    initPlanets();
}


AssetManager::~AssetManager(){

}


void AssetManager::objectsInit(){
    /* Warning: redundant object loading, this is fine now, as in the future the terrain 
       collision will be built from heightmap data. I don't really mind loading the objects
       twice, but in the future, if we have kinematic triangle meshes that need to be loaded
       from the disk, this should only be done once. */

    //btQuaternion quat;
   /* std::unique_ptr<Model> terrain_model(new Model("../data/terrain.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, m_frustum, m_render_context, math::vec3(0.75, 0.75, 0.75)));
    std::unique_ptr<iv_array> array(new iv_array);
    std::unique_ptr<btGImpactMeshShape> shape;

    load_bullet_trimesh(array, shape, std::string("../data/terrain.dae"));

    quat.setEuler(0, 0, 0);
    std::shared_ptr<Kinematic> ground = std::make_shared<Kinematic>(terrain_model.get(), m_physics, 
                                                                    static_cast<btCollisionShape*>(shape.get()), btScalar(0.0), 1);
    ground->setCollisionGroup(CG_DEFAULT | CG_KINEMATIC);
    ground->setCollisionFilters(~CG_RAY_EDITOR_RADIAL & ~CG_RAY_EDITOR_SELECT);
    ground->addBody(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0), quat);
    ground->setTrimesh(array); // pass array ownership to the kinematic

    m_kinematics.emplace_back(ground);
    m_models.push_back(std::move(terrain_model));
    m_collision_shapes.push_back(std::move(shape));*/

    /*std::unique_ptr<Model> terrain_model(new Model("../data/sphere.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, m_frustum, m_render_context, math::vec3(0.75, 0.75, 0.75)));
    std::unique_ptr<btCollisionShape> sphere_shape(new btSphereShape(50.0));

    quat.setEuler(0, 0, 0);
    std::shared_ptr<Kinematic> ground = std::make_shared<Kinematic>(terrain_model.get(), m_physics, 
                                                                    sphere_shape.get(), btScalar(0.0), 1);
    ground->setCollisionGroup(CG_DEFAULT | CG_KINEMATIC);
    ground->setCollisionFilters(~CG_RAY_EDITOR_RADIAL & ~CG_RAY_EDITOR_SELECT);
    ground->addBody(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0), quat);
    ground->setMeshScale(50.f);

    m_kinematics.emplace_back(ground);
    m_models.push_back(std::move(terrain_model));
    m_collision_shapes.push_back(std::move(sphere_shape));*/
}


void AssetManager::initResources(){
    std::pair<ResourceIterator, bool> res;
    std::string resource_name;
    std::unique_ptr<Resource> resource;
    std::hash<std::string> str_hash; // size_t = 64bit?? change???

    resource_name = "liquid_oxygen";
    resource.reset(new Resource(resource_name, std::string(u8"Liquid Oxygen (LOx/O₂)"), RESOURCE_TYPE_OXIDIZER, RESOURCE_STATE_LIQUID, 1141.0, 60.0));
    resource->setId(str_hash(resource_name));
    res = m_resources.insert({str_hash(resource_name), std::move(resource)});

    if(!res.second){
        log("Failed to insert resource with id ", str_hash(resource_name), " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert resource with id " << str_hash(resource_name) << " (collided with " << res.first->first << ")" << std::endl;
    }

    resource_name = "liquid_hydrogen";
    resource.reset(new Resource(resource_name, std::string(u8"Liquid Hydrogen (LH₂)"), RESOURCE_TYPE_FUEL_LIQUID, RESOURCE_STATE_LIQUID, 70.99, 30.0));
    resource->setId(str_hash(resource_name));
    res = m_resources.insert({str_hash(resource_name), std::move(resource)});

    if(!res.second){
        log("Failed to insert resource with id ", str_hash(resource_name), " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert resource with id " << str_hash(resource_name) << " (collided with " << res.first->first << ")" << std::endl;
    }

    resource_name = "rp1";
    resource.reset(new Resource(resource_name, std::string(u8"Rocket Propellant-1 (RP-1)"), RESOURCE_TYPE_FUEL_LIQUID, RESOURCE_STATE_LIQUID, 70.99, 30.0));
    resource->setId(str_hash(resource_name));
    res = m_resources.insert({str_hash(resource_name), std::move(resource)});

    if(!res.second){
        log("Failed to insert resource with id ", str_hash(resource_name), " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert resource with id " << str_hash(resource_name) << " (collided with " << res.first->first << ")" << std::endl;
    }

    resource_name = "htpb";
    resource.reset(new Resource(resource_name, std::string(u8"Hydroxyl-terminated polybutadiene (HTPB)"), RESOURCE_TYPE_FUEL_SOLID, RESOURCE_STATE_SOLID, 913.0, 298.15));
    resource->setId(str_hash(resource_name));
    res = m_resources.insert({str_hash(resource_name), std::move(resource)});

    if(!res.second){
        log("Failed to insert resource with id ", str_hash(resource_name), " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert resource with id " << str_hash(resource_name) << " (collided with " << res.first->first << ")" << std::endl;
    }
}


void AssetManager::processCommandBuffers(bool physics_pause){
    // process buffers
    for(uint i=0; i < m_add_body_buffer.size(); i++){
        struct add_body_msg& msg = m_add_body_buffer.at(i);
        msg.part->addBody(msg.origin, msg.inertia, msg.rotation);
    }
    m_add_body_buffer.clear();

    for(uint i=0; i < m_apply_force_buffer.size(); i++){
        struct apply_force_msg& msg = m_apply_force_buffer.at(i);
        msg.part->m_body->applyForce(msg.force, msg.rel_pos);
    }
    m_apply_force_buffer.clear();

    for(uint i=0; i < m_set_motion_state_buffer.size(); i++){
        struct set_motion_state_msg& msg = m_set_motion_state_buffer.at(i);
        msg.object->setMotionState(msg.origin, msg.initial_rotation);
        if(physics_pause){
            m_physics->updateCollisionWorldSingleAABB(msg.object->m_body.get());
        }
    }
    m_set_motion_state_buffer.clear();

    for(uint i=0; i < m_build_constraint_subtree_buffer.size(); i++){
        m_build_constraint_subtree_buffer.at(i)->buildSubTreeConstraints(nullptr);
    }
    m_build_constraint_subtree_buffer.clear();

    for(uint i=0; i < m_add_constraint_buffer.size(); i++){
        struct add_contraint_msg& msg = m_add_constraint_buffer.at(i);
        msg.part->setParentConstraint(msg.constraint_uptr);
    }
    m_add_constraint_buffer.clear();

    for(uint i=0; i < m_set_mass_buffer.size(); i++){
        // this should be enough, also the constraints don't get removed
        btDiscreteDynamicsWorld* dynamics_world = m_physics->getDynamicsWorld();
        btVector3 inertia;
        btRigidBody* body = m_set_mass_buffer.at(i).part->m_body.get();

        dynamics_world->removeRigidBody(body);

        body->getCollisionShape()->calculateLocalInertia(m_set_mass_buffer.at(i).mass, inertia);
        body->setMassProps(m_set_mass_buffer.at(i).mass, inertia);

        dynamics_world->addRigidBody(body);
    }
    m_set_mass_buffer.clear();

    for(uint i=0; i < m_remove_part_constraint_buffer.size(); i++){
        m_remove_part_constraint_buffer.at(i)->removeParentConstraint();
    }
    m_remove_part_constraint_buffer.clear();

    for(uint i=0; i < m_add_vessel_buffer.size(); i++){
        m_active_vessels.insert({m_add_vessel_buffer.at(i)->getId(), m_add_vessel_buffer.at(i)});
    }
    m_add_vessel_buffer.clear();

    for(uint i=0; i < m_delete_subtree_buffer.size(); i++){
        m_delete_subtree_buffer.at(i)->removeBodiesSubtree();
    }
    m_delete_subtree_buffer.clear();
}


void AssetManager::clearSceneEditor(){
    SubTreeIterator it;
    VesselIterator it2;

    for(it=m_editor_subtrees.begin(); it != m_editor_subtrees.end(); it++){
        m_delete_subtree_buffer.emplace_back(std::static_pointer_cast<BasePart>(it->second->getSharedPtr()));
    }
    m_editor_subtrees.clear();

    if(m_editor_vessel.get()){
        m_delete_subtree_buffer.emplace_back(std::static_pointer_cast<BasePart>(m_editor_vessel->getRoot()->getSharedPtr()));
        m_editor_vessel.reset();
    }
}


void AssetManager::deleteObjectEditor(BasePart* part, std::uint32_t& vessel_id){
    m_delete_subtree_buffer.emplace_back(std::static_pointer_cast<BasePart>(part->getSharedPtr()));
    if(part->getVessel()){
        std::uint32_t vid = part->getVessel()->getId();

        if(vid == vessel_id){
            m_editor_vessel.reset();
            vessel_id = 0;
        }
        else{
            log("AssetManager::deleteObjectEditor: tried to delete a vessel from the editor but it's not the"
                "editor's vessel. (", vid, " != ", vessel_id, ")");
            std::cerr << "AssetManager::deleteObjectEditor: tried to delete a vessel from the editor but it's not the"
                "editor's vessel. (" << vid << " != " << vessel_id << ")" << std::endl;
        }
    }
    else{
        std::uint32_t stid = part->getUniqueId();
        m_editor_subtrees.erase(stid);
    }
}


void AssetManager::updateObjectBuffer(std::vector<object_transform>& buffer_, const dmath::vec3& cam_origin){
    btVector3 btv_cam_origin(cam_origin.v[0], cam_origin.v[1], cam_origin.v[2]);

    buffer_.clear();
    switch(m_app->getRenderState()){
        case RENDER_NOTHING:
            break;
        case RENDER_EDITOR:
            updateObjectBufferEditor(buffer_, btv_cam_origin);
            break;
        case RENDER_UNIVERSE:
            updateObjectBufferUniverse(buffer_, btv_cam_origin);
            break;
        default:
            std::cerr << "AssetManager::updateObjectBuffer: got an invalid render state value from BaseApp::getRenderState (" << m_app->getRenderState() << ")" << std::endl;
            log("AssetManager::updateObjectBuffer: got an invalid render state value from BaseApp::getRenderState (", m_app->getRenderState(), ")");
            return;
    }
}


void AssetManager::updateObjectBufferEditor(std::vector<object_transform>& buffer_, const btVector3& btv_cam_origin){
    // draw distance check is probably irrelevant in the editor
    SubTreeIterator it;

    for(it=m_editor_subtrees.begin(); it != m_editor_subtrees.end(); it++){
        it->second->addSubTreeToRenderBuffer(buffer_, btv_cam_origin);
    }

    if(m_editor_vessel.get()){
        m_editor_vessel->getRoot()->addSubTreeToRenderBuffer(buffer_, btv_cam_origin);
    }    

    for(uint i=0; i < m_symmetry_subtrees.size(); i++){
        m_symmetry_subtrees.at(i)->addSubTreeToRenderBuffer(buffer_, btv_cam_origin);
    }
}


void AssetManager::updateObjectBufferUniverse(std::vector<object_transform>& buffer_, const btVector3& btv_cam_origin){
    // missing: draw distance check
    VesselIterator it;

    for(it=m_active_vessels.begin(); it != m_active_vessels.end(); it++){
        it->second->getRoot()->addSubTreeToRenderBuffer(buffer_, btv_cam_origin);;
    }

    for(uint i=0; i < m_kinematics.size(); i++){
        addObjectBuffer(m_kinematics.at(i).get(), buffer_, btv_cam_origin);
    }

    for(uint i=0; i < m_objects.size(); i++){
        addObjectBuffer(m_objects.at(i).get(), buffer_, btv_cam_origin);
    }
}


void AssetManager::addObjectBuffer(Object* obj, std::vector<object_transform>& buffer_, const btVector3& btv_cam_origin){
    try{
        math::mat4 mat;
        double b_transform[16];

        obj->getRigidBodyTransformDouble(b_transform);
        b_transform[12] -= btv_cam_origin.getX();   // object is transformed wrt camera origin
        b_transform[13] -= btv_cam_origin.getY();
        b_transform[14] -= btv_cam_origin.getZ();
        std::copy(b_transform, b_transform + 16, mat.m);

        buffer_.emplace_back(std::move(obj->getSharedPtr()), mat);
    }
    catch(std::bad_weak_ptr& e){
        std::string name;
        obj->getFancyName(name);
        std::cout << "AssetManager::addObjectBuffer: Warning, weak ptr for object " << name << " with id " << obj->getBaseId() << '\n';
    }
}


void AssetManager::updatePlanetBuffer(std::vector<planet_transform>& buffer_){
    buffer_.clear();

    for(uint i=0; i < m_planets.size(); i++){
        buffer_.emplace_back(m_planets.at(i).get(), m_planets.at(i)->getTransform());
    }
}


void AssetManager::updateBuffers(){
    /*std::chrono::duration<double, std::micro> time;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end;*/
    const dmath::vec3& cam_origin = m_camera->getCamPosition();
    struct render_buffer* rbuf;
    buffer_manager which_buffer;

    if(m_buffers->last_updated == buffer_2 || m_buffers->last_updated == none){
        if(m_buffers->buffer_1.buffer_lock.try_lock()){
            rbuf = &m_buffers->buffer_1;
            which_buffer = buffer_1;
        }
        else{
            m_buffers->buffer_2.buffer_lock.lock();
            rbuf = &m_buffers->buffer_2;
            which_buffer = buffer_2;
        }
    }
    else{
        if(m_buffers->buffer_2.buffer_lock.try_lock()){
            rbuf = &m_buffers->buffer_2;
            which_buffer = buffer_2;
        }
        else{
            m_buffers->buffer_1.buffer_lock.lock();
            rbuf = &m_buffers->buffer_1;
            which_buffer = buffer_1;
        }
    }

    rbuf->view_mat = m_camera->getCenteredViewMatrix();
    updateObjectBuffer(rbuf->buffer, cam_origin);
    updatePlanetBuffer(rbuf->planet_buffer);
    rbuf->cam_origin = cam_origin;
    m_buffers->last_updated = which_buffer;
    rbuf->buffer_lock.unlock();

    /*end = std::chrono::steady_clock::now();
    time = end - start;
    std::cout << "copy time: " << time.count() << std::endl;*/
}


void AssetManager::updateVessels(){
    VesselIterator it;

    for(it=m_active_vessels.begin(); it != m_active_vessels.end(); it++){
        it->second->update();
    }
}


void AssetManager::cleanup(){
    VesselIterator it1;
    SubTreeIterator it2;

    // make sure all the command buffers are emptied before clearing everything
    processCommandBuffers(false);

    for(uint i=0; i < m_objects.size(); i++){
        m_objects.at(i)->removeBody();
    }

    if(m_editor_vessel.get()){
        m_editor_vessel->getRoot()->removeBodiesSubtree();
    }

    for(it2=m_editor_subtrees.begin(); it2 != m_editor_subtrees.end(); it2++){
        it2->second->removeBodiesSubtree();
    }

    for(uint i=0; i < m_symmetry_subtrees.size(); i++){
        m_symmetry_subtrees.at(i)->removeBodiesSubtree();
    }

    for(it1=m_active_vessels.begin(); it1 != m_active_vessels.end(); it1++){
        it1->second->getRoot()->removeBodiesSubtree();
    }
}


void AssetManager::updateKinematics(){
    for(uint i=0; i < m_kinematics.size(); i++){
        m_kinematics.at(i)->update();
    }
}


void AssetManager::initPlanets(){
    Planet::loadBases(m_frustum, m_render_context);

    std::unique_ptr<Planet> earth = std::make_unique<Planet>(m_render_context);
    m_planets.emplace_back(std::move(earth));
}


void AssetManager::updateCoMs(){
    VesselIterator it;

    for(it=m_active_vessels.begin(); it != m_active_vessels.end(); it++){
        it->second->updateCoM();
    }
}

