#include "App.hpp"


App::App() : BaseApp(){
    modelsInit();
    objectsInit();

    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_physics_pause = true;
    m_picked_obj = nullptr;
    m_buffers.last_updated = none;
}


App::App(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    modelsInit();
    objectsInit();

    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_physics_pause = true;
    m_picked_obj = nullptr;
    m_buffers.last_updated = none;
}


App::~App(){
}


void App::modelsInit(){
    m_cube_model.reset(new Model("../data/cube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), math::vec3(0.5, 0.0, 0.5)));
    m_terrain_model.reset(new Model("../data/bigcube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), math::vec3(0.75, 0.75, 0.75)));
    m_sphere_model.reset(new Model("../data/sphere.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), math::vec3(0.75, 0.75, 0.75)));
    m_cylinder_model.reset(new Model("../data/cylinder.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), math::vec3(0.25, 0.25, 0.25)));
}


void App::objectsInit(){
    btQuaternion quat;

    std::unique_ptr<btCollisionShape> cube_shape_ground(new btBoxShape(btVector3(btScalar(25.), btScalar(25.), btScalar(25.))));
    std::unique_ptr<btCollisionShape> cube_shape(new btBoxShape(btVector3(1,1,1)));
    std::unique_ptr<btCollisionShape> sphere_shape(new btSphereShape(btScalar(1)));
    std::unique_ptr<btCollisionShape> cube3m(new btBoxShape(btVector3(3,3,3)));
    std::unique_ptr<btCollisionShape> cylinder_shape(new btCylinderShape(btVector3(1,1,1)));

    quat.setEuler(0, 0, 0);
    Object* ground = new Object(m_terrain_model.get(), m_bt_wrapper.get(), cube_shape_ground.get(), btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(0.0));
    m_objects.push_back(std::move(std::unique_ptr<Object>(ground)));

    for(int i=0; i<10; i++){
        Object* cube = new Object(m_cube_model.get(), m_bt_wrapper.get(), cube_shape.get(), btVector3(0.0, 30.0+i*2.5, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(10.0));
        cube->setColor(math::vec3(0.0, 1.0, 0.0));
        m_objects.push_back(std::move(std::unique_ptr<Object>(cube)));
    }

    for(int i=0; i<10; i++){
        // testing attachment points
        BasePart* cube = new BasePart(m_cube_model.get(), m_bt_wrapper.get(), cube_shape.get(), btVector3(2.5, 30.0+i*2.5, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(10.0));
        cube->setColor(math::vec3(0.0, 0.0, 1.0));
        cube->setParentAttachmentPoint(btVector3(0.0, 1.0, 0.0), btVector3(0.0, 0.0, 0.0));
        cube->addAttachmentPoint(btVector3(1.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0));
        cube->addAttachmentPoint(btVector3(0.0, -1.0, 0.0), btVector3(0.0, 0.0, 0.0));
        cube->addAttachmentPoint(btVector3(1.0, 0.0, 1.0), btVector3(0.0, 0.0, 0.0));
        m_parts.push_back(std::move(std::unique_ptr<BasePart>(cube)));
    }

    m_collision_shapes.push_back(std::move(cube_shape_ground));
    m_collision_shapes.push_back(std::move(cube_shape));
    m_collision_shapes.push_back(std::move(sphere_shape));
    m_collision_shapes.push_back(std::move(cube3m));
    m_collision_shapes.push_back(std::move(cylinder_shape));
}


void App::run(){
    m_bt_wrapper->startSimulation(1.f / 60.f, 2);
    while (!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        m_camera->update();
        m_frustum->extractPlanes(m_camera->getViewMatrix(), m_camera->getProjMatrix(), false);

        // mouse pick test
        if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] == INPUT_MBUTTON_PRESS){
            if(!m_picked_obj){
                double mousey, mousex;
                math::vec3 ray_start_world, ray_end_world;
                Object* obj;

                m_input->getMousePos(mousex, mousey);
                m_camera->castRayMousePos(1000.f, ray_start_world, ray_end_world);

                obj = m_bt_wrapper->testRay(ray_start_world, ray_end_world);
                if(obj)
                    m_picked_obj = obj;
            }
            else{
                m_picked_obj->activate(true);
                m_picked_obj = nullptr;
            }
        }

        if(m_picked_obj){
            math::vec3 ray_start_world, ray_end_world;
            btVector3 ray_end_world_btv3;
            btQuaternion rotation;
            btTransform transform;
            
            m_camera->castRayMousePos(25.f, ray_start_world, ray_end_world);
            ray_end_world_btv3 = btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]);
            m_picked_obj->m_body->getMotionState()->getWorldTransform(transform);
            rotation = transform.getRotation();
            m_picked_obj->setMotionState(ray_end_world_btv3, rotation); // WARNING: MOST LIKELY NOT THREAD SAFE (sometimes throws "pure virtual method called")

            if(m_physics_pause)
                m_bt_wrapper->updateCollisionWorldSingleAABB(m_picked_obj->m_body.get()); // not thread safe
        }

        if(m_input->pressed_keys[GLFW_KEY_P] == INPUT_KEY_DOWN){
            m_physics_pause = !m_physics_pause;
            m_bt_wrapper->pauseSimulation(m_physics_pause);
        }

        // rendering
        m_render_context->setDebugOverlayPhysicsTimes(m_bt_wrapper->getAverageLoadTime(), m_bt_wrapper->getAverageSleepTime());

        if(m_physics_pause)
            m_render_context->render(true);
        else
            m_render_context->render(false);

        glfwSwapBuffers(m_window_handler->getWindow());
    }
    m_bt_wrapper->stopSimulation();
    m_window_handler->terminate();
}

