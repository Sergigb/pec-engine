#ifndef APP_HPP
#define APP_HPP

#include <memory>
#include <vector>
#include <mutex>

#include "maths_funcs.hpp"
#include "Model.hpp"
#include "Object.hpp"
#include "BaseApp.hpp"

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
        bool m_physics_pause;
        Object* m_picked_obj;

        void modelsInit();
        void objectsInit();
    public:
        App();
        App(int gl_width, int gl_height);
        ~App();

        void run();
};


#endif
