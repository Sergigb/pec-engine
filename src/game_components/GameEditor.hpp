#ifndef GAME_EDITOR_HPP
#define GAME_EDITOR_HPP

#include <memory>
#include <cstdint>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>

#include "../core/maths_funcs.hpp"


class Object;
class EditorGUI;
class FontAtlas;
class BaseApp;
class BasePart;
class RenderContext;
class WindowHandler;
class AssetManager;
class Input;
class Frustum;
class Camera;
class Physics;
class Player;
class EditorGUI;
class EditorRenderer;

struct thread_monitor;

#define EXIT_LAUNCH 1
#define EXIT_MAIN_MENU 2


class GameEditor{
    private:
        // game state
        bool m_physics_pause, m_clear_scene, m_delete_current, m_exit_editor;
        Object* m_picked_obj;
        std::uint32_t m_vessel_id;
        uint m_symmetry_sides;
        bool m_radial_align;

        // GUI
        int m_gui_action;
        std::unique_ptr<EditorGUI> m_editor_gui;
        BasePart* m_highlight_part;

        // Editor renderer
        std::unique_ptr<EditorRenderer> m_renderer;

        // application default font atlas
        const FontAtlas* m_def_font_atlas;

        // quit code
        int m_exit_code;

        // here be something derived from BaseApp
        BaseApp* m_app;

        // uninitialized for now
        RenderContext* m_render_context;
        WindowHandler* m_window_handler;
        AssetManager* m_asset_manager;
        Input* m_input;
        Frustum* m_frustum;
        Camera* m_camera;
        Physics* m_physics;
        Player* m_player;

        struct thread_monitor* m_thread_monitor;

        // main loop
        void synchPreStep();
        void synchPostStep();
        void wakePhysics();
        void waitPhysics();

        //void placeClonedSubtreesRadial(BasePart* parent);
        void processInput();
        void processGUI();
        void placeClonedSubtreesOnClones(BasePart* closest, btTransform& transform_final, std::vector<BasePart*>& clone_to);
        void pickAttachedObject(BasePart* part);
        void hitPointAlign(btVector3& hit_point_world, btVector3& hit_normal_world, btTransform& parent_transform);
        void clearSymmetrySubtrees();
        void createSymmetrySubtrees();
        void createConstraint(BasePart* part, BasePart* parent, btTransform frame);
        void onRightMouseButton();
        void getUserRotation(btQuaternion& rotation, const btQuaternion& current_rotation);
        void logic();
        void clearScene();
        void deleteCurrent();
        void getClosestAtt(float& closest_dist, math::vec4& closest_att_point_world, BasePart*& closest, BasePart* part);
        void placeSubTree(float closest_dist, math::vec4& closest_att_point_world, BasePart* closest, BasePart* part);
        void pickObject();
        void init();
        void setPartHighlight();
    public:
        GameEditor(BaseApp* app, FontAtlas* font_atlas);
        ~GameEditor();

        int start();
};


#endif
