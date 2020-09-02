#ifndef ASSET_MANAGER_HPP
#define ASSET_MANAGER_HPP

#include <memory>
#include <vector>
#include <cstdint>
#include <map>


class Model;
class Object;
class btCollisionShape;
class BasePart;
class Vessel;
class RenderContext;
class Frustum;
class BtWrapper;

struct set_motion_state_msg;
struct add_contraint_msg;
struct add_body_msg;


class AssetManager{
    private:
        void loadParts();
        void objectsInit();
        void modelsInit();

        // this will be a vector
        std::unique_ptr<Model> m_engine;
        std::unique_ptr<Model> m_tank;
        std::unique_ptr<Model> m_tank2;
        std::unique_ptr<Model> m_terrain_model;
        std::unique_ptr<Model> m_com_module;

        RenderContext* m_render_context;
        const Frustum* m_frustum;
        BtWrapper* m_bt_wrapper;
    public:
        std::vector<std::shared_ptr<Object>> m_objects;
        std::vector<std::unique_ptr<btCollisionShape>> m_collision_shapes;
        std::map<std::uint32_t, std::unique_ptr<BasePart>> m_master_parts;

        // command buffers
        std::vector<struct set_motion_state_msg> m_set_motion_state_buffer;
        std::vector<BasePart*> m_remove_part_constraint_buffer;
        std::vector<struct add_contraint_msg> m_add_constraint_buffer;
        std::vector<struct add_body_msg> m_add_body_buffer;

        std::map<std::uint32_t, std::shared_ptr<BasePart>> m_editor_subtrees;
        std::map<std::uint32_t, std::shared_ptr<Vessel>> m_editor_vessels;

        AssetManager(RenderContext* render_context, const Frustum* frustum, BtWrapper* bt_wrapper);
        ~AssetManager();

        void processCommandBuffers(bool physics_pause);

        /*Editor methods, only call them before calling logic() and waking up the physics thread*/
        void clearSceneEditor();
        void deleteObjectEditor(BasePart* part, std::uint32_t& vessel_id);
};


#endif