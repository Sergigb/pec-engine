#ifndef ASSET_MANAGER_HPP
#define ASSET_MANAGER_HPP

#include <memory>
#include <vector>
#include <cstdint>
#include <unordered_map>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "AssetManagerInterface.hpp"
#include "assets_utils.hpp"
#include "buffers.hpp"
#include "Planet.hpp"


class Model;
class Object;
class btCollisionShape;
class BasePart;
class Vessel;
class RenderContext;
class Frustum;
class Physics;
class Camera;
class Resource;
class AssetManager;
class Kinematic;
class BaseApp;


class AssetManager{
    private:
        void objectsInit();
        void initResources();
        void initPlanets();

        RenderContext* m_render_context;
        const Frustum* m_frustum;
        Physics* m_physics;
        const Camera* m_camera;
        BaseApp* m_app;

        friend class AssetManagerInterface;
        AssetManagerInterface m_asset_manager_interface;

        friend void load_parts(AssetManager& asset_manager);

        // render buffers
        struct render_buffers* m_buffers;
        void updateObjectBuffer(std::vector<object_transform>& buffer_, const dmath::vec3& cam_origin);
        void updatePlanetBuffer(std::vector<planet_transform>& buffer_);
    public:
        std::vector<std::unique_ptr<btCollisionShape>> m_collision_shapes;
        std::vector<std::unique_ptr<Model>> m_models;
        std::unordered_map<std::uint32_t, std::unique_ptr<BasePart>> m_master_parts;
        std::unordered_map<std::uint32_t, std::unique_ptr<Resource>> m_resources;

        // command buffers
        std::vector<struct set_motion_state_msg> m_set_motion_state_buffer;
        std::vector<BasePart*> m_remove_part_constraint_buffer;
        std::vector<struct add_contraint_msg> m_add_constraint_buffer;
        std::vector<struct add_body_msg> m_add_body_buffer;
        std::vector<std::shared_ptr<Vessel>> m_add_vessel_buffer;
        std::vector<struct apply_force_msg> m_apply_force_buffer;
        std::vector<struct set_mass_props_msg> m_set_mass_buffer;
        std::vector<std::shared_ptr<BasePart>> m_delete_subtree_buffer;
        std::vector<BasePart*> m_build_constraint_subtree_buffer;

        // editor objects
        std::unordered_map<std::uint32_t, std::shared_ptr<BasePart>> m_editor_subtrees;
        std::shared_ptr<Vessel> m_editor_vessel;
        std::vector<std::shared_ptr<BasePart>> m_symmetry_subtrees;

        // universe
        std::vector<std::shared_ptr<Object>> m_objects;
        std::vector<std::shared_ptr<Kinematic>> m_kinematics;
        std::vector<std::unique_ptr<Planet>> m_planets; // too simple, it will be more complex in the future when I have a proper planetary system
        std::unordered_map<std::uint32_t, std::shared_ptr<Vessel>> m_active_vessels;

        //AssetManager(RenderContext* render_context, const Frustum* frustum, Physics* physics, render_buffers* buff_manager, Camera* camera);
        AssetManager(BaseApp* app);
        ~AssetManager();

        void processCommandBuffers(bool physics_pause);
        void updateBuffers();

        /*Editor methods, only call them before calling logic() and waking up the physics thread*/
        void clearSceneEditor();
        void deleteObjectEditor(BasePart* part, std::uint32_t& vessel_id);

        /* Simulation vessels update, this may not need to be here since m_editor_vessels is public */
        void updateVessels();
        void updateKinematics(); // testing
        void updateCoMs(); // synch call

        void cleanup();
};


#endif