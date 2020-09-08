#include "AssetManager.hpp"
#include "RenderContext.hpp"
#include "Frustum.hpp"
#include "BtWrapper.hpp"
#include "Vessel.hpp"
#include "Camera.hpp"


AssetManager::AssetManager(RenderContext* render_context, const Frustum* frustum, BtWrapper* bt_wrapper, render_buffers* buff_manager, Camera* camera){
    m_render_context = render_context;
    m_frustum = frustum;
    m_bt_wrapper = bt_wrapper;
    m_buffers = buff_manager;
    m_camera = camera;

    m_asset_manager_interface = AssetManagerInterface(this);

    modelsInit(); // temporal
    objectsInit();
    loadParts();
}


AssetManager::~AssetManager(){

}


void AssetManager::modelsInit(){
    m_engine.reset(new Model("../data/engine.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, m_render_context, math::vec3(0.25, 0.25, 0.25)));
    m_tank.reset(new Model("../data/tank.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, m_render_context, math::vec3(0.25, 0.25, 0.25)));
    m_tank2.reset(new Model("../data/tank2.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, m_render_context, math::vec3(0.25, 0.25, 0.25)));
    m_terrain_model.reset(new Model("../data/bigcube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, m_render_context, math::vec3(0.75, 0.75, 0.75)));
    m_com_module.reset(new Model("../data/capsule.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, m_render_context, math::vec3(0.75, 0.75, 0.75)));
    m_separator.reset(new Model("../data/separator.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, m_render_context, math::vec3(0.75, 0.75, 0.75)));
}


void AssetManager::objectsInit(){
    btQuaternion quat;
    std::unique_ptr<btCollisionShape> cube_shape_ground(new btBoxShape(btVector3(btScalar(25.), btScalar(25.), btScalar(25.))));

    quat.setEuler(0, 0, 0);
    std::shared_ptr<Object> ground = std::make_shared<Object>(m_terrain_model.get(), m_bt_wrapper, cube_shape_ground.get(), btScalar(0.0), 1);
    ground->setCollisionGroup(CG_DEFAULT | CG_KINEMATIC);
    ground->setCollisionFilters(~CG_RAY_EDITOR_RADIAL & ~CG_RAY_EDITOR_SELECT); // by default, do not collide with radial attach. ray tests
    ground->addBody(btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0), quat);
    m_objects.emplace_back(ground);

    m_collision_shapes.push_back(std::move(cube_shape_ground));
}


void AssetManager::loadParts(){
    // not really "loading" for now but whatever
    // for now I'm just adding cubes as parts, just for testing
    btQuaternion quat;
    quat.setEuler(0, 0, 0);
    typedef std::map<std::uint32_t, std::unique_ptr<BasePart>>::iterator map_iterator;
    std::pair<map_iterator, bool> res;

    std::unique_ptr<btCollisionShape> cube_shape(new btBoxShape(btVector3(1,1,1)));
    std::unique_ptr<btCollisionShape> cylinder_shape(new btCylinderShape(btVector3(1,1,1)));
    std::unique_ptr<btCollisionShape> cylinder_shape_tank(new btCylinderShape(btVector3(1.0, 5.0, 1.0)));
    std::unique_ptr<btCollisionShape> cylinder_shape_tank2(new btCylinderShape(btVector3(1.0, 2.5, 1.0)));
    std::unique_ptr<btCollisionShape> cylinder_shape_separator(new btCylinderShape(btVector3(1.0, 0.065, 1.0)));
    std::unique_ptr<btCollisionShape> sphere_shape(new btSphereShape(1.0));
    std::unique_ptr<btCollisionShape> cone(new btConeShape(0.5, 1.70));

    std::unique_ptr<BasePart> separator(new BasePart(m_separator.get(), m_bt_wrapper, cylinder_shape_separator.get(), btScalar(10.0), 555, &m_asset_manager_interface));
    separator->setColor(math::vec3(0.75, 0.75, 0.75));
    separator->setParentAttachmentPoint(math::vec3(0.0, 0.065, 0.0), math::vec3(0.0, 0.0, 0.0));
    separator->addAttachmentPoint(math::vec3(0.0, -0.065, 0.0), math::vec3(0.0, 0.0, 0.0));
    separator->setName(std::string("separator_") + std::to_string(555));
    separator->setFancyName("Separator");
    separator->setCollisionGroup(CG_DEFAULT | CG_PART);
    separator->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
    separator->setProperties(PART_DECOUPLES_CHILDS | PART_DECOUPLES);

    res = m_master_parts.insert({555, std::move(separator)});

    if(!res.second){
        log("Failed to inset part with id ", 555, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to inset part with id " << 555 << " (collided with " << res.first->first << ")" << std::endl;
    }

    std::unique_ptr<BasePart> com_module(new BasePart(m_com_module.get(), m_bt_wrapper, cone.get(), btScalar(10.0), 444, &m_asset_manager_interface));
    com_module->setColor(math::vec3(0.75, 0.75, 0.75));
    com_module->setParentAttachmentPoint(math::vec3(0.0, 0.605, 0.0), math::vec3(0.0, 0.0, 0.0));
    com_module->addAttachmentPoint(math::vec3(0.0, -0.653, 0.0), math::vec3(0.0, 0.0, 0.0));
    com_module->setName(std::string("command_module_") + std::to_string(444));
    com_module->setFancyName("Command Module");
    com_module->setCollisionGroup(CG_DEFAULT | CG_PART);
    com_module->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
    com_module->setProperties(PART_IS_CM);

    res = m_master_parts.insert({444, std::move(com_module)});

    if(!res.second){
        log("Failed to inset part with id ", 444, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to inset part with id " << 444 << " (collided with " << res.first->first << ")" << std::endl;
    }

    std::unique_ptr<BasePart> tank(new BasePart(m_tank.get(), m_bt_wrapper, cylinder_shape_tank.get(), btScalar(10.0), 222, &m_asset_manager_interface));
    tank->setColor(math::vec3(0.75, 0.75, 0.75));
    tank->setParentAttachmentPoint(math::vec3(0.0, 5.0, 0.0), math::vec3(0.0, 0.0, 0.0));
    tank->addAttachmentPoint(math::vec3(0.0, -5.0, 0.0), math::vec3(0.0, 0.0, 0.0));
    tank->setName(std::string("tank_") + std::to_string(222));
    tank->setFancyName("Tank");
    tank->setCollisionGroup(CG_DEFAULT | CG_PART);
    tank->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);

    res = m_master_parts.insert({222, std::move(tank)});

    if(!res.second){
        log("Failed to inset part with id ", 222, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to inset part with id " << 222 << " (collided with " << res.first->first << ")" << std::endl;
    }

    std::unique_ptr<BasePart> tank2(new BasePart(m_tank2.get(), m_bt_wrapper, cylinder_shape_tank2.get(), btScalar(10.0), 111, &m_asset_manager_interface));
    tank2->setColor(math::vec3(0.75, 0.75, 0.75));
    tank2->setParentAttachmentPoint(math::vec3(0.0, 2.5, 0.0), math::vec3(0.0, 0.0, 0.0));
    tank2->addAttachmentPoint(math::vec3(0.0, -2.5, 0.0), math::vec3(0.0, 0.0, 0.0));
    tank2->setName(std::string("tank_") + std::to_string(111));
    tank2->setFancyName("Tank 2");
    tank2->setCollisionGroup(CG_DEFAULT | CG_PART);
    tank2->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);

    res = m_master_parts.insert({111, std::move(tank2)});

    if(!res.second){
        log("Failed to inset part with id ", 111, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to inset part with id " << 111 << " (collided with " << res.first->first << ")" << std::endl;
    }

    std::unique_ptr<BasePart> engine(new BasePart(m_engine.get(), m_bt_wrapper, cylinder_shape.get(), btScalar(10.0), 333, &m_asset_manager_interface));
    engine->setColor(math::vec3(0.75, 0.75, 0.75));
    engine->setParentAttachmentPoint(math::vec3(0.0, 0.43459, 0.0), math::vec3(0.0, 0.0, 0.0));
    engine->addAttachmentPoint(math::vec3(0.0, -0.848, 0.0), math::vec3(0.0, 0.0, 0.0));
    engine->setName(std::string("engine_") + std::to_string(333));
    engine->setFancyName("Engine");
    engine->setCollisionGroup(CG_DEFAULT | CG_PART);
    engine->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
    engine->setProperties(PART_HAS_ENGINE);

    res = m_master_parts.insert({333, std::move(engine)});

    if(!res.second){
        log("Failed to inset part with id ", 333, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to inset part with id " << 333 << " (collided with " << res.first->first << ")" << std::endl;
    }

    m_collision_shapes.push_back(std::move(sphere_shape));
    m_collision_shapes.push_back(std::move(cube_shape));
    m_collision_shapes.push_back(std::move(cylinder_shape));
    m_collision_shapes.push_back(std::move(cylinder_shape_tank));
    m_collision_shapes.push_back(std::move(cylinder_shape_tank2));
    m_collision_shapes.push_back(std::move(cone));
    m_collision_shapes.push_back(std::move(cylinder_shape_separator));
}


void AssetManager::processCommandBuffers(bool physics_pause){
    // process buffers
    for(uint i=0; i < m_set_motion_state_buffer.size(); i++){
        struct set_motion_state_msg& msg = m_set_motion_state_buffer.at(i);
        msg.object->setMotionState(msg.origin, msg.initial_rotation);
        if(physics_pause){
            m_bt_wrapper->updateCollisionWorldSingleAABB(msg.object->m_body.get());
        }
    }
    m_set_motion_state_buffer.clear();

    for(uint i=0; i < m_add_constraint_buffer.size(); i++){
        struct add_contraint_msg& msg = m_add_constraint_buffer.at(i);
        msg.part->setParentConstraint(msg.constraint_uptr);
    }
    m_add_constraint_buffer.clear();

    for(uint i=0; i < m_add_body_buffer.size(); i++){
        struct add_body_msg& msg = m_add_body_buffer.at(i);
        msg.part->addBody(msg.origin, msg.inertia, msg.rotation);
    }
    m_add_body_buffer.clear();

    for(uint i=0; i < m_remove_part_constraint_buffer.size(); i++){
        m_remove_part_constraint_buffer.at(i)->removeParentConstraint();
    }
    m_remove_part_constraint_buffer.clear();

    for(uint i=0; i < m_add_vessel_buffer.size(); i++){
        m_editor_vessels.insert({m_add_vessel_buffer.at(i)->getId(), m_add_vessel_buffer.at(i)});
    }
    m_add_vessel_buffer.clear();
}


void AssetManager::clearSceneEditor(){
    std::map<std::uint32_t, std::shared_ptr<BasePart>>::iterator it;
    std::map<std::uint32_t, std::shared_ptr<Vessel>>::iterator it2;

    for(it=m_editor_subtrees.begin(); it != m_editor_subtrees.end(); it++){
        it->second->setRenderIgnoreSubTree();
    }
    m_editor_subtrees.clear();

    for(it2=m_editor_vessels.begin(); it2 != m_editor_vessels.end(); it2++){
        it2->second->getRoot()->setRenderIgnoreSubTree();
    }
    m_editor_vessels.clear();
}


void AssetManager::deleteObjectEditor(BasePart* part, std::uint32_t& vessel_id){
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


void AssetManager::updateBuffer(std::vector<object_transform>* buffer_){
    const btDiscreteDynamicsWorld* dynamics_world = m_bt_wrapper->getDynamicsWorld();
    const btCollisionObjectArray& col_object_array = dynamics_world->getCollisionObjectArray();
    dmath::vec3 cam_origin = m_camera->getCamPosition();
    btVector3 btv_cam_origin(cam_origin.v[0], cam_origin.v[1], cam_origin.v[2]);

    buffer_->clear();
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

            buffer_->emplace_back(object_transform{obj->getSharedPtr(), mat});
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
            updateBuffer(&m_buffers->buffer1);
            m_buffers->last_updated = buffer_1;
            m_buffers->buffer1_lock.unlock();
        }
        else{
            m_buffers->buffer2_lock.lock(); // very unlikely to not get the lock
            m_buffers->view_mat2 = m_camera->getCenteredViewMatrix();
            updateBuffer(&m_buffers->buffer2);
            m_buffers->last_updated = buffer_2;
            m_buffers->buffer2_lock.unlock();
        }
    }
    else{
        if(m_buffers->buffer2_lock.try_lock()){
            m_buffers->view_mat2 = m_camera->getCenteredViewMatrix();
            updateBuffer(&m_buffers->buffer2);
            m_buffers->last_updated = buffer_2;
            m_buffers->buffer2_lock.unlock();
        }
        else{
            m_buffers->view_mat1 = m_camera->getCenteredViewMatrix();
            m_buffers->buffer1_lock.lock();
            updateBuffer(&m_buffers->buffer1);
            m_buffers->last_updated = buffer_1;
            m_buffers->buffer1_lock.unlock();
        }
    }
    /*end = std::chrono::steady_clock::now();
    time = end - start;
    std::cout << "copy time: " << time.count() << std::endl;*/
}

