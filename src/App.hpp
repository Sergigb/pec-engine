#ifndef APP_HPP
#define APP_HPP

#include <memory>
#include <cstdint>

#include "BaseApp.hpp"


#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


class Object;
class EditorGUI;
class FontAtlas;


#define MAX_SYMMETRY_SIDES 8
#define SIDE_ANGLE_STEP double(M_PI / MAX_SYMMETRY_SIDES)


class App : public BaseApp{
    private:
        // game state
        bool m_physics_pause, m_clear_scene, m_delete_current;
        Object* m_picked_obj;
        std::uint32_t m_vessel_id;
        uint m_symmetry_sides;
        bool m_radial_align;

        // time
        std::chrono::duration<double, std::micro> m_elapsed_time;

        // GUI
        std::unique_ptr<EditorGUI> m_editor_gui;
        int m_gui_action;

        // application default font atlas
        std::unique_ptr<FontAtlas> m_def_font_atlas;

        void pickAttachedObject(BasePart* part);
        void hitPointAlign(btVector3& hit_point_world, btVector3& hit_normal_world, btTransform& parent_transform);
        void clearSymmetrySubtrees();
        void createSymmetrySubtrees();
        void createConstraint(BasePart* part, BasePart* parent, btTransform frame);
        void onLeftMouseButton();
        void getUserRotation(btQuaternion& rotation, const btQuaternion& current_rotation);
        void logic();
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
