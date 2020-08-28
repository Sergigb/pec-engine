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
        // game state
        bool m_physics_pause, m_clear_scene, m_delete_current;
        Object* m_picked_obj;

        std::uint32_t m_vessel_id;
        std::unique_ptr<Vessel> m_editor_vessel;

        // time
        std::chrono::duration<double, std::micro> m_elapsed_time;

        // GUI
        std::unique_ptr<EditorGUI> m_editor_gui;
        int m_gui_action;

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        void onLeftMouseButton();
        void getUserRotation(btQuaternion& rotation, const btQuaternion& current_rotation);
        void modelsInit();
        void objectsInit();
        void loadParts();
        void logic();
        void processCommandBuffers();
        void clearScene();
        void deleteCurrent();
        void getClosestAtt(float& closest_dist, math::vec4& closest_att_point_world, BasePart*& closest, BasePart* part);
        void placeSubTree(float closest_dist, math::vec4& closest_att_point_world, BasePart* closest, BasePart* part);
        void pickObject();
        void init();
    public:
        App();
        App(int gl_width, int gl_height);
        ~App();

        void run();
};


#endif
