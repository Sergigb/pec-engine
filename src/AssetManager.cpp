#include "AssetManager.hpp"
#include "RenderContext.hpp"
#include "Frustum.hpp"
#include "BtWrapper.hpp"
#include "buffers.hpp"
#include "Vessel.hpp"


AssetManager::AssetManager(RenderContext* render_context, const Frustum* frustum, BtWrapper* bt_wrapper){
    m_render_context = render_context;
    m_frustum = frustum;
    m_bt_wrapper = bt_wrapper;

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
    std::unique_ptr<btCollisionShape> sphere_shape(new btSphereShape(1.0));
    std::unique_ptr<btCollisionShape> cone(new btConeShape(0.5, 1.70));

    std::unique_ptr<BasePart> com_module(new BasePart(m_com_module.get(), m_bt_wrapper, cone.get(), btScalar(10.0), 444));
    com_module->setColor(math::vec3(0.75, 0.75, 0.75));
    com_module->setParentAttachmentPoint(math::vec3(0.0, 0.605, 0.0), math::vec3(0.0, 0.0, 0.0));
    com_module->addAttachmentPoint(math::vec3(0.0, -0.653, 0.0), math::vec3(0.0, 0.0, 0.0));
    com_module->setName(std::string("command_module_") + std::to_string(444));
    com_module->setFancyName("Command Module");
    com_module->setCollisionGroup(CG_DEFAULT | CG_PART);
    com_module->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);

    res = m_master_parts.insert({444, std::move(com_module)});

    if(!res.second){
        log("Failed to inset part with id ", 444, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to inset part with id " << 444 << " (collided with " << res.first->first << ")" << std::endl;
    }

    std::unique_ptr<BasePart> tank(new BasePart(m_tank.get(), m_bt_wrapper, cylinder_shape_tank.get(), btScalar(10.0), 222));
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

    std::unique_ptr<BasePart> tank2(new BasePart(m_tank2.get(), m_bt_wrapper, cylinder_shape_tank2.get(), btScalar(10.0), 111));
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

    std::unique_ptr<BasePart> engine(new BasePart(m_engine.get(), m_bt_wrapper, cylinder_shape.get(), btScalar(10.0), 333));
    engine->setColor(math::vec3(0.75, 0.75, 0.75));
    engine->setParentAttachmentPoint(math::vec3(0.0, 0.43459, 0.0), math::vec3(0.0, 0.0, 0.0));
    engine->addAttachmentPoint(math::vec3(0.0, -0.848, 0.0), math::vec3(0.0, 0.0, 0.0));
    engine->setName(std::string("engine_") + std::to_string(333));
    engine->setFancyName("Engine");
    engine->setCollisionGroup(CG_DEFAULT | CG_PART);
    engine->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);

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

