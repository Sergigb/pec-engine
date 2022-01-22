#ifndef ASSET_MANAGER_HPP
#define ASSET_MANAGER_HPP

#include <memory>
#include <vector>
#include <cstdint>
#include <unordered_map>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "AssetManagerInterface.hpp"
#include "utils/assets_utils.hpp"
#include "buffers.hpp"


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
class Planet;
class PlanetarySystem;


typedef std::unordered_map<std::uint32_t, std::shared_ptr<BasePart>> SubTreeMap;
typedef std::unordered_map<std::uint32_t, std::shared_ptr<Vessel>> VesselMap;
typedef std::unordered_map<std::uint32_t, std::unique_ptr<Resource>> ResourceMap;
typedef std::unordered_map<std::uint32_t, std::unique_ptr<BasePart>> BasePartMap;


/*
 * Class used to manage assets (planets/parts/resources etc) and buffers, performs basic tasks such
 * as updating render and command buffers (required for multithreading) or deleting/updating assets. 
 * This class exposes most of its members, so it should only be visible to core classes that may
 * need it, but not to the assets. AssetManagerInterface is used instead to interact with this 
 * class (for example, adding commands to the command buffers).
 */

class AssetManager{
    private:
        /* 
         * Old method called to load some stuff that's not used anymore, will be gone in the future
         */
        void objectsInit();

        RenderContext* m_render_context;
        const Frustum* m_frustum;
        Physics* m_physics;
        const Camera* m_camera;
        BaseApp* m_app;

        /* Interface class for parts */
        friend class AssetManagerInterface;
        AssetManagerInterface m_asset_manager_interface;

        friend void load_parts(AssetManager& asset_manager);

        /* render buffers */
        struct render_buffers* m_buffers;

        /* 
         * Methods used to update render buffers once updateBuffers is called. These functions
         * are called depending on the render state of the app.
         */
        void updateObjectBuffer(std::vector<object_transform>& buffer_, const dmath::vec3& cam_origin);
        void updatePlanetBuffer(std::vector<planet_transform>& buffer_);
        void updateObjectBufferEditor(std::vector<object_transform>& buffer_, const btVector3& btv_cam_origin);
        void updateObjectBufferUniverse(std::vector<object_transform>& buffer_, const btVector3& btv_cam_origin);
        void addObjectBuffer(Object* obj, std::vector<object_transform>& buffer_, const btVector3& btv_cam_origin);
        void updateViewMat(struct render_buffer* rbuf) const;
    public:
        std::vector<std::unique_ptr<btCollisionShape>> m_collision_shapes;
        std::vector<std::unique_ptr<Model>> m_models;
        BasePartMap m_master_parts;
        ResourceMap m_resources;

        /* command buffers */
        std::vector<struct set_motion_state_msg> m_set_motion_state_buffer;
        std::vector<BasePart*> m_remove_part_constraint_buffer;
        std::vector<struct add_contraint_msg> m_add_constraint_buffer;
        std::vector<struct add_body_msg> m_add_body_buffer;
        std::vector<std::shared_ptr<Vessel>> m_add_vessel_buffer;
        std::vector<struct apply_force_msg> m_apply_force_buffer;
        std::vector<struct set_mass_props_msg> m_set_mass_buffer;
        std::vector<std::shared_ptr<BasePart>> m_delete_subtree_buffer;
        std::vector<BasePart*> m_build_constraint_subtree_buffer;

        /* editor objects */
        SubTreeMap m_editor_subtrees;
        std::shared_ptr<Vessel> m_editor_vessel;
        std::vector<std::shared_ptr<BasePart>> m_symmetry_subtrees;

        /* universe */
        std::vector<std::shared_ptr<Object>> m_objects;
        std::vector<std::shared_ptr<Kinematic>> m_kinematics;
        std::unique_ptr<PlanetarySystem> m_planetary_system;
        VesselMap m_active_vessels;

        AssetManager(BaseApp* app);
        ~AssetManager();

        /* 
         * Used to process the command buffers, must be called by the app from the main thread
         * when the physics thread is not running.
         * 
         * @physics_pause: wether the physics is paused (and thus Bullet has not stepped), in order
         * to update AABBs.
         */
        void processCommandBuffers(bool physics_pause);

        /* 
         * Called to update the render buffers, must be called when the physics thread is not 
         * running.
         */
        void updateBuffers();

        /*
         * Clears the editor scene (main vessel and subtrees), must be called with the physics
         * thread is not running.
         */
        void clearSceneEditor();

        /*
         * Deletes a subtree from the editor. The part should be the root of a subtree, if the
         * part is the root of the editor vessel the editor vessel gets deleted. This method should
         * be re-written.
         *
         * @part: pointer to the root part of the subtree to be deleted, should be the root of a 
         * subtree in m_editor_subtrees or the root of m_editor_vessel.
         * @vessel_id: id of the editor's vessel, should not be required and will be deleted in the
         * future.
         */
        void deleteObjectEditor(BasePart* part, std::uint32_t& vessel_id);

        /* 
         * Calls the update method of all the vessels, this should be called when the physics 
         * thread is running, the vessel's and part's update methods should use the command buffers
         * via AssetManagerInterface to interact with the AssetManager (see BasePart's and Vessel's
         * update method)
         */
        void updateVessels();

        /*
         * Old testing function for kinematics, but will probably not be used, as kinematics will
         * be updated from the Planet class
         */
        void updateKinematics();

        /*
         * Updates the center of mass of all the active vessels, should be called when the physics
         * thread is stopped to avoid data races while retrieving the position of the objects.
         */
        void updateCoMs();

        /*
         * Loading methods, call these depending on what the application needs. Note that you will
         * probably need to load the resources if you want to have parts.
         */
        void loadResources();
        void loadParts();
        void loadStarSystem();

        /*
         * Cleans everything (vessels, subtrees and buffers), call when the application is about to
         * exit.
         */
        void cleanup();
};


#endif
