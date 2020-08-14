#ifndef APP_HPP
#define APP_HPP

#include <memory>
#include <vector>
#include <mutex>
#include <string>
#include <cstdint>

#include "maths_funcs.hpp"
#include "Model.hpp"
#include "Object.hpp"
#include "BaseApp.hpp"
#include "EditorGUI.hpp"
#include "buffers.hpp"
#include "Vessel.hpp"


#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


class App : public BaseApp{
    private:
        // some models for testing
        std::unique_ptr<Model> m_cube_model;
        std::unique_ptr<Model> m_terrain_model;
        std::unique_ptr<Model> m_sphere_model;
        std::unique_ptr<Model> m_cylinder_model;

        // game state
        bool m_physics_pause, m_clear_scene;
        Object* m_picked_obj;

        std::vector<std::unique_ptr<btCollisionShape>> m_collision_shapes;

        //...
        std::vector<std::shared_ptr<Object>> m_objects;
        std::vector<std::shared_ptr<BasePart>> m_subtrees;
        std::map<std::uint32_t, std::shared_ptr<Vessel>> m_vessels;
        std::uint32_t m_vessel_id;

        // editor vessel
        std::unique_ptr<Vessel> m_vessel;

        // parts are copied from here
        std::map<std::uint32_t, std::unique_ptr<BasePart>> m_master_parts;

        // command buffers
        std::vector<struct set_motion_state_msg> m_set_motion_state_buffer;
        std::vector<BasePart*> m_remove_part_constraint_buffer;
        std::vector<struct add_contraint_msg> m_add_constraint_buffer;
        std::vector<struct add_body_msg> m_add_body_buffer;

        std::chrono::duration<double, std::micro> m_elapsed_time;

        // GUI testing
        std::unique_ptr<EditorGUI> m_editor_gui;
        int m_gui_action;

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        void modelsInit();
        void objectsInit();
        void loadParts();
        void logic();
        void processCommandBuffers();
        void clearScene();
        void getClosestAtt(float& closest_dist, math::vec4& closest_att_point_world, BasePart*& closest, BasePart* part);
        void placeSubTree(float closest_dist, math::vec4& closest_att_point_world, BasePart* closest, BasePart* part);
        void pickObject();
    public:
        App();
        App(int gl_width, int gl_height);
        ~App();

        void run();
};


#endif
