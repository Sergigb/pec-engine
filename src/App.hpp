#ifndef APP_HPP
#define APP_HPP

#include <memory>
#include <vector>
#include <mutex>

#include "maths_funcs.hpp"
#include "Model.hpp"
#include "Object.hpp"
#include "BaseApp.hpp"
#include "EditorGUI.hpp"

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


struct set_motion_state_msg{
    Object* object;
    btVector3 origin;
    btQuaternion initial_rotation;
};


struct add_contraint_msg{
    BasePart* part;
    std::unique_ptr<btTypedConstraint> constraint_uptr;
};


class App : public BaseApp{
    private:
        // some models for testing
        std::unique_ptr<Model> m_cube_model;
        std::unique_ptr<Model> m_terrain_model;
        std::unique_ptr<Model> m_sphere_model;
        std::unique_ptr<Model> m_cylinder_model;

        // game state
        bool m_physics_pause;
        Object* m_picked_obj;

        std::vector<std::unique_ptr<btCollisionShape>> m_collision_shapes;
        std::vector<std::unique_ptr<Object>> m_objects;

        /* Note: this is a tricky situation. If BasePart inherits from Object, why not
        include the parts in m_objects (and use virtual functions if needed)? As now,
        I want to have parts in a different vector for when we are in the editor and we
        need to draw the attachment points. I dont want to include opengl code in BasePart
        or Object, so the render context should be the one calling the opengl functions
        and draw the attachment points (and we need to know that the Object is actually 
        a BasePart to call the corresponding method). Also, I want to avoid using dynamic
        casts on m_objects (ew).

        This will change when we add the object trees, kinematic objects, and other more 
        complex crap.*/
        std::vector<std::unique_ptr<BasePart>> m_parts;

        // queues
        std::vector<struct set_motion_state_msg> m_set_motion_state_queue;
        std::vector<BasePart*> m_remove_part_constraint_queue;
        std::vector<struct add_contraint_msg> m_add_constraint_queue;

        std::chrono::duration<double, std::micro> m_elapsed_time;

        // GUI testing
        std::unique_ptr<EditorGUI> m_editor_gui;

        void modelsInit();
        void objectsInit();
        void logic();
    public:
        App();
        App(int gl_width, int gl_height);
        ~App();

        void run();
};


#endif
