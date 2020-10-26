#include "assets_utils.hpp"
//#include "BasePart.hpp"
#include "RenderContext.hpp"


void load_parts(AssetManager& asset_manager){
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

    std::unique_ptr<Model> engine_model(new Model("../data/engine.dae", nullptr, asset_manager.m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.25, 0.25, 0.25)));
    std::unique_ptr<Model> tank_model(new Model("../data/tank.dae", nullptr, asset_manager.m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.25, 0.25, 0.25)));
    std::unique_ptr<Model> tank2_model(new Model("../data/tank2.dae", nullptr, asset_manager.m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.25, 0.25, 0.25)));
    std::unique_ptr<Model> com_module_model(new Model("../data/capsule.dae", nullptr, asset_manager.m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.75, 0.75, 0.75)));
    std::unique_ptr<Model> separator_model(new Model("../data/separator.dae", nullptr, asset_manager.m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), asset_manager.m_frustum, asset_manager.m_render_context, math::vec3(0.75, 0.75, 0.75)));

    std::unique_ptr<BasePart> separator(new BasePart(separator_model.get(), asset_manager.m_bt_wrapper, cylinder_shape_separator.get(), btScalar(10.0), 555, &asset_manager.m_asset_manager_interface));
    separator->setColor(math::vec3(0.75, 0.75, 0.75));
    separator->setParentAttachmentPoint(math::vec3(0.0, 0.065, 0.0), math::vec3(0.0, 0.0, 0.0));
    separator->addAttachmentPoint(math::vec3(0.0, -0.065, 0.0), math::vec3(0.0, 0.0, 0.0));
    separator->setName(std::string("separator_") + std::to_string(555));
    separator->setFancyName("Separator");
    separator->setCollisionGroup(CG_DEFAULT | CG_PART);
    separator->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
    separator->setProperties(PART_DECOUPLES_CHILDS | PART_DECOUPLES);

    res = asset_manager.m_master_parts.insert({555, std::move(separator)});

    if(!res.second){
        log("Failed to insert part with id ", 555, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert part with id " << 555 << " (collided with " << res.first->first << ")" << std::endl;
    }

    std::unique_ptr<BasePart> com_module(new BasePart(com_module_model.get(), asset_manager.m_bt_wrapper, cone.get(), btScalar(10.0), 444, &asset_manager.m_asset_manager_interface));
    com_module->setColor(math::vec3(0.75, 0.75, 0.75));
    com_module->setParentAttachmentPoint(math::vec3(0.0, 0.605, 0.0), math::vec3(0.0, 0.0, 0.0));
    com_module->addAttachmentPoint(math::vec3(0.0, -0.653, 0.0), math::vec3(0.0, 0.0, 0.0));
    com_module->setName(std::string("command_module_") + std::to_string(444));
    com_module->setFancyName("Command Module");
    com_module->setCollisionGroup(CG_DEFAULT | CG_PART);
    com_module->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
    com_module->setProperties(PART_IS_CM);

    res = asset_manager.m_master_parts.insert({444, std::move(com_module)});

    if(!res.second){
        log("Failed to insert part with id ", 444, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert part with id " << 444 << " (collided with " << res.first->first << ")" << std::endl;
    }

    std::unique_ptr<BasePart> tank(new BasePart(tank_model.get(), asset_manager.m_bt_wrapper, cylinder_shape_tank.get(), btScalar(10.0), 222, &asset_manager.m_asset_manager_interface));
    tank->setColor(math::vec3(0.75, 0.75, 0.75));
    tank->setParentAttachmentPoint(math::vec3(0.0, 5.0, 0.0), math::vec3(0.0, 0.0, 0.0));
    tank->addAttachmentPoint(math::vec3(0.0, -5.0, 0.0), math::vec3(0.0, 0.0, 0.0));
    tank->setName(std::string("tank_") + std::to_string(222));
    tank->setFancyName("Tank");
    tank->setCollisionGroup(CG_DEFAULT | CG_PART);
    tank->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);

    res = asset_manager.m_master_parts.insert({222, std::move(tank)});

    if(!res.second){
        log("Failed to insert part with id ", 222, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert part with id " << 222 << " (collided with " << res.first->first << ")" << std::endl;
    }

    std::unique_ptr<BasePart> tank2(new BasePart(tank2_model.get(), asset_manager.m_bt_wrapper, cylinder_shape_tank2.get(), btScalar(10.0), 111, &asset_manager.m_asset_manager_interface));
    tank2->setColor(math::vec3(0.75, 0.75, 0.75));
    tank2->setParentAttachmentPoint(math::vec3(0.0, 2.5, 0.0), math::vec3(0.0, 0.0, 0.0));
    tank2->addAttachmentPoint(math::vec3(0.0, -2.5, 0.0), math::vec3(0.0, 0.0, 0.0));
    tank2->setName(std::string("tank_") + std::to_string(111));
    tank2->setFancyName("Tank 2");
    tank2->setCollisionGroup(CG_DEFAULT | CG_PART);
    tank2->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);

    res = asset_manager.m_master_parts.insert({111, std::move(tank2)});

    if(!res.second){
        log("Failed to insert part with id ", 111, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert part with id " << 111 << " (collided with " << res.first->first << ")" << std::endl;
    }

    std::unique_ptr<BasePart> engine(new BasePart(engine_model.get(), asset_manager.m_bt_wrapper, cylinder_shape.get(), btScalar(10.0), 333, &asset_manager.m_asset_manager_interface));
    engine->setColor(math::vec3(0.75, 0.75, 0.75));
    engine->setParentAttachmentPoint(math::vec3(0.0, 0.43459, 0.0), math::vec3(0.0, 0.0, 0.0));
    engine->addAttachmentPoint(math::vec3(0.0, -0.848, 0.0), math::vec3(0.0, 0.0, 0.0));
    engine->setName(std::string("engine_") + std::to_string(333));
    engine->setFancyName("Engine");
    engine->setCollisionGroup(CG_DEFAULT | CG_PART);
    engine->setCollisionFilters(~CG_RAY_EDITOR_RADIAL);
    engine->setProperties(PART_HAS_ENGINE);

    res = asset_manager.m_master_parts.insert({333, std::move(engine)});

    if(!res.second){
        log("Failed to insert part with id ", 333, " (collided with ", res.first->first, ")");
        std::cerr << "Failed to insert part with id " << 333 << " (collided with " << res.first->first << ")" << std::endl;
    }

    asset_manager.m_collision_shapes.push_back(std::move(sphere_shape));
    asset_manager.m_collision_shapes.push_back(std::move(cube_shape));
    asset_manager.m_collision_shapes.push_back(std::move(cylinder_shape));
    asset_manager.m_collision_shapes.push_back(std::move(cylinder_shape_tank));
    asset_manager.m_collision_shapes.push_back(std::move(cylinder_shape_tank2));
    asset_manager.m_collision_shapes.push_back(std::move(cone));
    asset_manager.m_collision_shapes.push_back(std::move(cylinder_shape_separator));

    asset_manager.m_models.push_back(std::move(engine_model));
    asset_manager.m_models.push_back(std::move(tank_model));
    asset_manager.m_models.push_back(std::move(tank2_model));
    asset_manager.m_models.push_back(std::move(com_module_model));
    asset_manager.m_models.push_back(std::move(separator_model));
}





