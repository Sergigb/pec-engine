#ifndef ASSET_MANAGER_HPP
#define ASSET_MANAGER_HPP

#include <memory>
#include <vector>
#include <cstdint>
#include <map>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "AssetManagerInterface.hpp"
#include "assets_utils.hpp"
#include "buffers.hpp"


class Model;
class Object;
class btCollisionShape;
class BasePart;
class Vessel;
class RenderContext;
class Frustum;
class BtWrapper;
class Camera;
class Resource;
class AssetManager;


class AssetManager{
    private:
        void objectsInit();
        void initResources();

        RenderContext* m_render_context;
        const Frustum* m_frustum;
        BtWrapper* m_bt_wrapper;
        const Camera* m_camera;

        friend class AssetManagerInterface;
        AssetManagerInterface m_asset_manager_interface;

        friend void load_parts(AssetManager& asset_manager);

        // render buffers
        struct render_buffers* m_buffers;
        void updateBuffer(std::vector<object_transform>* buffer_);
    public:
        std::vector<std::shared_ptr<Object>> m_objects;
        std::vector<std::unique_ptr<btCollisionShape>> m_collision_shapes;
        std::map<std::uint32_t, std::unique_ptr<BasePart>> m_master_parts;
        std::map<std::uint32_t, std::unique_ptr<Resource>> m_resources;
        std::vector<std::unique_ptr<Model>> m_models;

        // command buffers
        std::vector<struct set_motion_state_msg> m_set_motion_state_buffer;
        std::vector<BasePart*> m_remove_part_constraint_buffer;
        std::vector<struct add_contraint_msg> m_add_constraint_buffer;
        std::vector<struct add_body_msg> m_add_body_buffer;
        std::vector<std::shared_ptr<Vessel>> m_add_vessel_buffer;
        std::vector<struct apply_force_msg> m_apply_force_buffer;
        std::vector<struct set_mass_props_msg> m_set_mass_buffer;

        std::map<std::uint32_t, std::shared_ptr<BasePart>> m_editor_subtrees;
        std::map<std::uint32_t, std::shared_ptr<Vessel>> m_editor_vessels;

        AssetManager(RenderContext* render_context, const Frustum* frustum, BtWrapper* bt_wrapper, render_buffers* buff_manager, Camera* camera);
        ~AssetManager();

        void processCommandBuffers(bool physics_pause);
        void updateBuffers();

        /*Editor methods, only call them before calling logic() and waking up the physics thread*/
        void clearSceneEditor();
        void deleteObjectEditor(BasePart* part, std::uint32_t& vessel_id);

        /* Simulation vessels update, this may not need to be here since m_editor_vessels is public */
        void updateVessels();
};


#endif