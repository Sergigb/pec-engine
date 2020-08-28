#include "AssetManager.hpp"
#include "RenderContext.hpp"
#include "Frustum.hpp"
#include "BtWrapper.hpp"
#include "buffers.hpp"


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
    m_cube_model.reset(new Model("../data/cube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, m_render_context,math::vec3(0.5, 0.0, 0.5)));
    m_terrain_model.reset(new Model("../data/bigcube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, m_render_context, math::vec3(0.75, 0.75, 0.75)));
    m_sphere_model.reset(new Model("../data/sphere.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, m_render_context, math::vec3(0.75, 0.75, 0.75)));
    m_cylinder_model.reset(new Model("../data/cylinder.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, m_render_context, math::vec3(0.25, 0.25, 0.25)));
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

    std::unique_ptr<btCollisionShape> cube_shape(new btBoxShape(btVector3(1,1,1)));

    int howmany = 35;
    for(int i=0; i<howmany; i++){
        std::uint32_t ID = i + 5; // change in the future to something else

        std::unique_ptr<BasePart> cube(new BasePart(m_cube_model.get(), m_bt_wrapper, cube_shape.get(), btScalar(10.0), ID));
        cube->setColor(math::vec3(1.0-(1./howmany)*i, 0.0, (1./howmany)*i));
        cube->setParentAttachmentPoint(math::vec3(0.0, 1.0, 0.0), math::vec3(0.0, 0.0, 0.0));
        cube->setFreeAttachmentPoint(math::vec3(-1.0, 0.0, 0.0), math::vec3(-1.0, 0.0, 0.0));
        cube->addAttachmentPoint(math::vec3(1.0, 0.0, 0.0), math::vec3(0.0, 0.0, 0.0));
        cube->addAttachmentPoint(math::vec3(0.0, -1.0, 0.0), math::vec3(0.0, 0.0, 0.0));
        cube->addAttachmentPoint(math::vec3(1.0, 0.0, 1.0), math::vec3(0.0, 0.0, 0.0));
        cube->setName(std::string("test_part_id_") + std::to_string(ID));
        cube->setFancyName(std::string("Placeholder object ") + std::to_string(ID));
        cube->setCollisionGroup(CG_DEFAULT | CG_PART);
        cube->setCollisionFilters(~CG_RAY_EDITOR_RADIAL); // by default, do not collide with radial attach. ray tests

        typedef std::map<std::uint32_t, std::unique_ptr<BasePart>>::iterator map_iterator;
        std::pair<map_iterator, bool> res = m_master_parts.insert({ID, std::move(cube)});

        if(!res.second){
            log("Failed to inset part with id ", ID, " (collided with ", res.first->first, ")");
            std::cerr << "Failed to inset part with id " << ID << " (collided with " << res.first->first << ")" << std::endl;
        }
    }

    std::unique_ptr<btCollisionShape> cylinder_shape(new btCylinderShape(btVector3(1,1,1)));

    std::unique_ptr<BasePart> cylinder(new BasePart(m_cylinder_model.get(), m_bt_wrapper, cylinder_shape.get(), btScalar(10.0), 100));
    cylinder->setColor(math::vec3(1.0, 0.0, 1.0));
    cylinder->setParentAttachmentPoint(math::vec3(0.0, 1.0, 0.0), math::vec3(0.0, 0.0, 0.0));
    cylinder->setFreeAttachmentPoint(math::vec3(1.0, 0.0, 0.0), math::vec3(1.0, 0.0, 0.0));
    cylinder->addAttachmentPoint(math::vec3(1.0, 0.0, 0.0), math::vec3(0.0, 0.0, 0.0));
    cylinder->setName(std::string("cylinder") + std::to_string(100));
    cylinder->setFancyName(std::string("Cylinder ") + std::to_string(100));
    cylinder->setCollisionGroup(CG_DEFAULT | CG_PART);
    cylinder->setCollisionFilters(~CG_RAY_EDITOR_RADIAL); // by default, do not collide with radial attach. ray tests

    typedef std::map<std::uint32_t, std::unique_ptr<BasePart>>::iterator map_iterator;
    std::pair<map_iterator, bool> res = m_master_parts.insert({100, std::move(cylinder)});

    if(!res.second){
        log("Failed to inset part with id ", 100, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to inset part with id " << 100 << " (collided with " << res.first->first << ")" << std::endl;
    }

    std::unique_ptr<btCollisionShape> sphere_shape(new btSphereShape(1.0));

    std::unique_ptr<BasePart> sphere(new BasePart(m_sphere_model.get(), m_bt_wrapper, sphere_shape.get(), btScalar(10.0), 101));
    sphere->setColor(math::vec3(1.0, 0.0, 1.0));
    sphere->setParentAttachmentPoint(math::vec3(0.0, 1.0, 0.0), math::vec3(0.0, 0.0, 0.0));
    sphere->setFreeAttachmentPoint(math::vec3(0.0, 0.0, 1.0), math::vec3(0.0, 0.0, 1.0));
    sphere->addAttachmentPoint(math::vec3(0.0, -1.0, 0.0), math::vec3(0.0, 0.0, 0.0));
    sphere->setName(std::string("sphere_") + std::to_string(101));
    sphere->setFancyName(std::string("Sphere ") + std::to_string(101));
    sphere->setCollisionGroup(CG_DEFAULT | CG_PART);
    sphere->setCollisionFilters(~CG_RAY_EDITOR_RADIAL); // by default, do not collide with radial attach. ray tests

    typedef std::map<std::uint32_t, std::unique_ptr<BasePart>>::iterator map_iterator;
    res = m_master_parts.insert({101, std::move(sphere)});

    if(!res.second){
        log("Failed to inset part with id ", 101, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to inset part with id " << 101 << " (collided with " << res.first->first << ")" << std::endl;
    }

    m_collision_shapes.push_back(std::move(sphere_shape));
    m_collision_shapes.push_back(std::move(cube_shape));
    m_collision_shapes.push_back(std::move(cylinder_shape));
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

