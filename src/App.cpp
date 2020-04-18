#include "App.hpp"


App::App(){
    init(640, 480);
    modelsInit();
    objectsInit();
    m_render_context->setObjectVector(&m_objects);

    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));
}


App::App(int gl_width, int gl_height){
    init(gl_width, gl_height);
    modelsInit();
    objectsInit();
    m_render_context->setObjectVector(&m_objects);

    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));
}


App::~App(){
}


void App::init(int gl_width, int gl_height){
    m_input.reset(new Input());
    m_camera.reset(new Camera(math::vec3(-0.0f, 50.0f, 50.0f), 67.0f, (float)gl_width / (float)gl_height , 0.1f, 100000.0f, m_input.get()));
    m_camera->setSpeed(10.f);
    m_window_handler.reset(new WindowHandler(gl_width, gl_height, m_input.get(), m_camera.get()));
    m_camera->setWindowHandler(m_window_handler.get());
    m_frustum.reset(new Frustum());
    m_render_context.reset(new RenderContext(m_camera.get(), m_window_handler.get()));
    m_bt_wrapper.reset(new BtWrapper(btVector3(0, -9.81, 0)));

    m_physics_pause = true;
    m_picked_obj = nullptr;
    m_last_updated = none;
}


void App::modelsInit(){
    m_cube_model.reset(new Model("../data/cube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), math::vec3(0.5, 0.0, 0.5)));
    m_terrain_model.reset(new Model("../data/bigcube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), math::vec3(0.75, 0.75, 0.75)));
    m_sphere_model.reset(new Model("../data/sphere.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), math::vec3(0.75, 0.75, 0.75)));
}


void App::objectsInit(){
    btQuaternion quat;

    std::unique_ptr<btCollisionShape> cube_shape_ground(new btBoxShape(btVector3(btScalar(25.), btScalar(25.), btScalar(25.))));
    std::unique_ptr<btCollisionShape> cube_shape(new btBoxShape(btVector3(1,1,1)));
    std::unique_ptr<btCollisionShape> sphere_shape(new btSphereShape(btScalar(1)));
    std::unique_ptr<btCollisionShape> cube3m(new btBoxShape(btVector3(3,3,3)));

    quat.setEuler(0, 0, 0);
    Object* ground = new Object(m_terrain_model.get(), m_bt_wrapper.get(), cube_shape_ground.get(), btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(0.0));

    quat.setEuler(20, 50, 0);
    Object* cube1 = new Object(m_cube_model.get(), m_bt_wrapper.get(), cube3m.get(), btVector3(0.0, 40.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(1.0));
    cube1->setMeshScale(3.0);
    cube1->setColor(math::vec3(1.0, 0.0, 0.0));

    quat.setEuler(0, 0, 0);
    Object* cube2 = new Object(m_cube_model.get(), m_bt_wrapper.get(), cube_shape.get(), btVector3(10.5, 30.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(30.0));
    cube2->setColor(math::vec3(0.5, 0.75, 0.0));

    quat.setEuler(0, 0, 0);
    Object* cube3 = new Object(m_cube_model.get(), m_bt_wrapper.get(), cube_shape.get(), btVector3(12.5, 30.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(30.0));
    cube3->setColor(math::vec3(0.5, 0.75, 0.0));

    m_objects.push_back(std::move(std::unique_ptr<Object>(ground)));
    m_objects.push_back(std::move(std::unique_ptr<Object>(cube1)));
    m_objects.push_back(std::move(std::unique_ptr<Object>(cube2)));
    m_objects.push_back(std::move(std::unique_ptr<Object>(cube3)));

    // trying constraints
    btVector3 pivot_a(1.0f, 0.0f, 0.0f);
    btVector3 pivot_b(-1.0f, 0.0f, -0.0f);

    btVector3 axis_a(0.0f, 1.0f, 0.0f );
    btVector3 axis_b(0.0f, 1.0f, 0.0f );

    btHingeConstraint* hingeConstraint = new btHingeConstraint(*cube2->getRigidBody(), *cube3->getRigidBody(), pivot_a, pivot_b, axis_a, axis_b, false);

    hingeConstraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 0);
    hingeConstraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 1);
    hingeConstraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 2);
    hingeConstraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 3);
    hingeConstraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 4);
    hingeConstraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 5);

    hingeConstraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 0);
    hingeConstraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 1);
    hingeConstraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 2);
    hingeConstraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 3);
    hingeConstraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 4);
    hingeConstraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 5);

    hingeConstraint->setLimit(-0, 0);

    m_bt_wrapper->addConstraint(hingeConstraint, true);

    for(int i=0; i<10; i++){
        Object* cube = new Object(m_cube_model.get(), m_bt_wrapper.get(), cube_shape.get(), btVector3(0.0, 55.0 + (i+1)*5, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(10.0));
        cube->setColor(math::vec3(1.0, 0.0, 0.0));
        m_objects.push_back(std::move(std::unique_ptr<Object>(cube)));
    }

    for(int i=0; i<10; i++){
        Object* sphere = new Object(m_sphere_model.get(), m_bt_wrapper.get(), sphere_shape.get(), btVector3(5.0, 55.0 + (i+1)*5, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(10.0));
        sphere->setColor(math::vec3(0.0, 1.0, 0.0));
        m_objects.push_back(std::move(std::unique_ptr<Object>(sphere)));
    }

    m_collision_shapes.push_back(std::move(cube_shape_ground));
    m_collision_shapes.push_back(std::move(cube_shape));
    m_collision_shapes.push_back(std::move(sphere_shape));
    m_collision_shapes.push_back(std::move(cube3m));
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
            btQuaternion rotation;
            btVector3 ray_end_world_btv3;
            
            m_camera->castRayMousePos(25.f, ray_start_world, ray_end_world);
            ray_end_world_btv3 = btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]);
            rotation.setEuler(0, 0, 0);
            m_picked_obj->setMotionState(ray_end_world_btv3, rotation);

            if(m_physics_pause)
                m_bt_wrapper->updateCollisionWorldSingleAABB(m_picked_obj->getRigidBody()); // not thread safe
        }

        if(m_input->pressed_keys[GLFW_KEY_P]){
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

