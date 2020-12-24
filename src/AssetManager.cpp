#include <iostream>
#include <functional>

#include "AssetManager.hpp"
#include "RenderContext.hpp"
#include "Frustum.hpp"
#include "BtWrapper.hpp"
#include "Vessel.hpp"
#include "Camera.hpp"
#include "Resource.hpp"
#include "Model.hpp"
#include "Object.hpp"
#include "BasePart.hpp"
#include "log.hpp"
#include "Kinematic.hpp"


typedef std::map<std::uint32_t, std::shared_ptr<BasePart>>::iterator SubTreeIterator;
typedef std::map<std::uint32_t, std::shared_ptr<Vessel>>::iterator VesselIterator;
typedef std::map<std::uint32_t, std::unique_ptr<Resource>>::iterator ResourceIterator;


AssetManager::AssetManager(RenderContext* render_context, const Frustum* frustum, BtWrapper* bt_wrapper, render_buffers* buff_manager, Camera* camera){
    m_render_context = render_context;
    m_frustum = frustum;
    m_bt_wrapper = bt_wrapper;
    m_buffers = buff_manager;
    m_camera = camera;

    m_asset_manager_interface = AssetManagerInterface(this);

    initResources();
    objectsInit();
    load_parts(*this);
}


AssetManager::~AssetManager(){

}


void AssetManager::objectsInit(){
    // testing terrain stuff
    btQuaternion quat;
    std::unique_ptr<btCollisionShape> cube_shape_ground(new btBoxShape(btVector3(btScalar(25.), btScalar(25.), btScalar(25.))));
    std::unique_ptr<Model> cube_model(new Model("../data/bigcube.dae", nullptr, SHADER_PHONG_BLINN_NO_TEXTURE, m_frustum, m_render_context, math::vec3(0.75, 0.75, 0.75)));
    
    quat.setEuler(0, 0, 0);
    std::shared_ptr<Kinematic> ground = std::make_shared<Kinematic>(cube_model.get(), m_bt_wrapper, cube_shape_ground.get(), btScalar(0.0), 1);
    ground->setCollisionGroup(CG_DEFAULT | CG_KINEMATIC);
    ground->setCollisionFilters(~CG_RAY_EDITOR_RADIAL & ~CG_RAY_EDITOR_SELECT); // by default, do not collide with radial attach. ray tests
    ground->addBody(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0), quat);
    m_kinematics.emplace_back(ground);

    m_models.push_back(std::move(cube_model));
    m_collision_shapes.push_back(std::move(cube_shape_ground));
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
            m_bt_wrapper->updateCollisionWorldSingleAABB(msg.object->m_body.get());
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
        btDiscreteDynamicsWorld* dynamics_world = m_bt_wrapper->getDynamicsWorld();
        btVector3 inertia;
        btRigidBody* body = m_set_mass_buffer.at(i).part->m_body.get();

        dynamics_world->removeRigidBody(body);

        body->getCollisionShape()->calculateLocalInertia(m_set_mass_buffer.at(i).mass, inertia);
        body->setMassProps(m_set_mass_buffer.at(i).mass, inertia);

        dynamics_world->addRigidBody(body);
    }

    for(uint i=0; i < m_remove_part_constraint_buffer.size(); i++){
        m_remove_part_constraint_buffer.at(i)->removeParentConstraint();
    }
    m_remove_part_constraint_buffer.clear();

    for(uint i=0; i < m_add_vessel_buffer.size(); i++){
        m_editor_vessels.insert({m_add_vessel_buffer.at(i)->getId(), m_add_vessel_buffer.at(i)});
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
        it->second->setRenderIgnoreSubTree();
        m_delete_subtree_buffer.emplace_back(std::static_pointer_cast<BasePart>(it->second->getSharedPtr()));
    }
    m_editor_subtrees.clear();

    for(it2=m_editor_vessels.begin(); it2 != m_editor_vessels.end(); it2++){
        it2->second->getRoot()->setRenderIgnoreSubTree();
        m_delete_subtree_buffer.emplace_back(std::static_pointer_cast<BasePart>(it2->second->getRoot()->getSharedPtr()));
    }
    m_editor_vessels.clear();
}


void AssetManager::deleteObjectEditor(BasePart* part, std::uint32_t& vessel_id){
    m_delete_subtree_buffer.emplace_back(std::static_pointer_cast<BasePart>(part->getSharedPtr()));
    if(part->getVessel()){
        std::uint32_t vid = part->getVessel()->getId();

        m_editor_vessels.at(vid)->getRoot()->setRenderIgnoreSubTree();
        m_editor_vessels.erase(vid);
        if(vessel_id == vid){
            vessel_id = 0;
        }
    }
    else{
        std::uint32_t stid = part->getUniqueId();
        m_editor_subtrees.at(stid)->setRenderIgnoreSubTree();
        m_editor_subtrees.erase(stid);
    }
}


void AssetManager::updateBuffer(std::vector<object_transform>& buffer_){
    const btDiscreteDynamicsWorld* dynamics_world = m_bt_wrapper->getDynamicsWorld();
    const btCollisionObjectArray& col_object_array = dynamics_world->getCollisionObjectArray();
    dmath::vec3 cam_origin = m_camera->getCamPosition();
    btVector3 btv_cam_origin(cam_origin.v[0], cam_origin.v[1], cam_origin.v[2]);

    buffer_.clear();
    std::vector<object_transform>::iterator it;
    for(int i=0; i<col_object_array.size(); i++){
        Object* obj = static_cast<Object *>(col_object_array.at(i)->getUserPointer());
        if(obj->renderIgnore()){ // ignore if the object should be destroyed
            continue;
        }
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
        catch(std::bad_weak_ptr& e) {
            std::string name;
            obj->getFancyName(name);
            std::cout << "AssetManager::updateBuffer - Warning, weak ptr for object " << name << " with id " << obj->getBaseId() << '\n';
        }
    }
}


void AssetManager::updateBuffers(){
    /*std::chrono::duration<double, std::micro> time;
    std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point end;*/

    if(m_buffers->last_updated == buffer_2 || m_buffers->last_updated == none){
        if(m_buffers->buffer1_lock.try_lock()){
            m_buffers->view_mat1 = m_camera->getCenteredViewMatrix();
            updateBuffer(m_buffers->buffer1);
            m_buffers->last_updated = buffer_1;
            m_buffers->buffer1_lock.unlock();
        }
        else{
            m_buffers->buffer2_lock.lock(); // very unlikely to not get the lock
            m_buffers->view_mat2 = m_camera->getCenteredViewMatrix();
            updateBuffer(m_buffers->buffer2);
            m_buffers->last_updated = buffer_2;
            m_buffers->buffer2_lock.unlock();
        }
    }
    else{
        if(m_buffers->buffer2_lock.try_lock()){
            m_buffers->view_mat2 = m_camera->getCenteredViewMatrix();
            updateBuffer(m_buffers->buffer2);
            m_buffers->last_updated = buffer_2;
            m_buffers->buffer2_lock.unlock();
        }
        else{
            m_buffers->view_mat1 = m_camera->getCenteredViewMatrix();
            m_buffers->buffer1_lock.lock();
            updateBuffer(m_buffers->buffer1);
            m_buffers->last_updated = buffer_1;
            m_buffers->buffer1_lock.unlock();
        }
    }
    /*end = std::chrono::steady_clock::now();
    time = end - start;
    std::cout << "copy time: " << time.count() << std::endl;*/
}


void AssetManager::updateVessels(){
    VesselIterator it;

    for(it=m_editor_vessels.begin(); it != m_editor_vessels.end(); it++){
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

    for(it1=m_editor_vessels.begin(); it1 != m_editor_vessels.end(); it1++){
        it1->second->getRoot()->removeBodiesSubtree();
    }

    for(it2=m_editor_subtrees.begin(); it2 != m_editor_subtrees.end(); it2++){
        it2->second->removeBodiesSubtree();
    }

    for(uint i=0; i < m_symmetry_subtrees.size(); i++){
        m_symmetry_subtrees.at(i)->removeBodiesSubtree();
    }
}

