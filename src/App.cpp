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
    for (int j = 0; j < m_collision_shapes.size(); j++){
        btCollisionShape* shape = m_collision_shapes[j];
        m_collision_shapes[j] = 0;
        delete shape;
    }

    for(uint i=0; i < m_objects.size(); i++){
        delete m_objects.at(i);
    }
    m_objects.clear();

    delete m_input;
    delete m_camera;
    delete m_window_handler;
    delete m_frustum;
    delete m_render_context;
    delete m_bt_wrapper;

    delete m_cube_model;
    delete m_terrain_model;
    delete m_sphere_model;
}


void App::init(int gl_width, int gl_height){
    m_input = new Input();
    m_camera = new Camera(math::vec3(-0.0f, 50.0f, 50.0f), 67.0f, (float)gl_width / (float)gl_height , 0.1f, 100000.0f, m_input);
    m_camera->setSpeed(10.f);
    m_window_handler = new WindowHandler(gl_width, gl_height, m_input, m_camera);
    m_camera->setWindowHandler(m_window_handler);
    m_frustum = new Frustum();
    m_render_context = new RenderContext(m_camera, m_window_handler, &m_buffer1, &m_buffer2, &m_buffer1_lock, &m_buffer2_lock, &m_manager_lock, &m_last_updated);
    m_bt_wrapper = new BtWrapper(btVector3(0, -9.81, 0), &m_buffer1, &m_buffer2, &m_buffer1_lock, &m_buffer2_lock, &m_manager_lock, &m_last_updated);

    m_physics_pause = true;
    m_picked_obj = nullptr;
    m_last_updated = none;
}


void App::modelsInit(){
    m_cube_model = new Model("../data/cube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, math::vec3(0.5, 0.0, 0.5));
    m_terrain_model = new Model("../data/bigcube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, math::vec3(0.75, 0.75, 0.75));
    m_sphere_model = new Model("../data/sphere.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum, math::vec3(0.75, 0.75, 0.75));
}


void App::objectsInit(){
    btQuaternion quat;

    btCollisionShape* cube_shape_ground = new btBoxShape(btVector3(btScalar(25.), btScalar(25.), btScalar(25.))); // box for now, we need to try a mesh
    btCollisionShape* cube_shape = new btBoxShape(btVector3(1,1,1));
    btCollisionShape* sphere_shape = new btSphereShape(btScalar(1));
    btCollisionShape* cube3m = new btBoxShape(btVector3(3,3,3));

    m_collision_shapes.push_back(cube_shape);
    m_collision_shapes.push_back(cube_shape_ground);
    m_collision_shapes.push_back(sphere_shape);
    m_collision_shapes.push_back(cube3m);

    quat.setEuler(0, 0, 0);
    Object* ground = new Object(m_terrain_model, m_bt_wrapper, cube_shape_ground, btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(0.0));

    quat.setEuler(20, 50, 0);
    Object* cube1 = new Object(m_cube_model, m_bt_wrapper, cube3m, btVector3(0.0, 40.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(1.0));
    cube1->setMeshScale(3.0);
    cube1->setColor(math::vec3(1.0, 0.0, 0.0));

    quat.setEuler(0, 0, 0);
    Object* cube2 = new Object(m_cube_model, m_bt_wrapper, cube_shape, btVector3(10.5, 30.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(30.0));
    cube2->setColor(math::vec3(0.5, 0.75, 0.0));

    quat.setEuler(0, 0, 0);
    Object* cube3 = new Object(m_cube_model, m_bt_wrapper, cube_shape, btVector3(12.5, 30.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(30.0));
    cube3->setColor(math::vec3(0.5, 0.75, 0.0));

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

    for(int i=0; i<4000; i++){
        Object* cube = new Object(m_cube_model, m_bt_wrapper, cube_shape, btVector3(0.0, 55.0 + (i+1)*5, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(10.0));
        cube->setColor(math::vec3(1.0, 0.0, 0.0));
        m_objects.push_back(cube);
    }

    for(int i=0; i<10; i++){
        Object* sphere = new Object(m_sphere_model, m_bt_wrapper, sphere_shape, btVector3(5.0, 55.0 + (i+1)*5, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(10.0));
        sphere->setColor(math::vec3(0.0, 1.0, 0.0));
        m_objects.push_back(sphere);
    }

    m_objects.push_back(ground);
    m_objects.push_back(cube1);
    m_objects.push_back(cube2);
    m_objects.push_back(cube3);
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
            m_render_context->render(true);

        glfwSwapBuffers(m_window_handler->getWindow());
    }
    m_bt_wrapper->stopSimulation();
    m_window_handler->terminate();
}

