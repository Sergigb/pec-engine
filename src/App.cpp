#include "App.hpp"


App::App() : BaseApp(){
    modelsInit();
    objectsInit();

    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_physics_pause = true;
    m_picked_obj = nullptr;
    m_buffers.last_updated = none;

    m_render_context->setObjectVector(&m_objects);
    m_render_context->setPartVector(&m_parts);
}


App::App(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    modelsInit();
    objectsInit();

    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_physics_pause = true;
    m_picked_obj = nullptr;
    m_buffers.last_updated = none;

    m_render_context->setObjectVector(&m_objects);
    m_render_context->setPartVector(&m_parts);
}


App::~App(){
}


void App::modelsInit(){
    m_cube_model.reset(new Model("../data/cube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), m_render_context.get(),math::vec3(0.5, 0.0, 0.5)));
    m_terrain_model.reset(new Model("../data/bigcube.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), m_render_context.get(), math::vec3(0.75, 0.75, 0.75)));
    m_sphere_model.reset(new Model("../data/sphere.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), m_render_context.get(), math::vec3(0.75, 0.75, 0.75)));
    m_cylinder_model.reset(new Model("../data/cylinder.dae", nullptr, m_render_context->getShader(SHADER_PHONG_BLINN_NO_TEXTURE), m_frustum.get(), m_render_context.get(), math::vec3(0.25, 0.25, 0.25)));
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
        BasePart* cube = new BasePart(m_cube_model.get(), m_bt_wrapper.get(), cube_shape.get(), btVector3(2.5, 30.0+i*5., 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(10.0));
        cube->setColor(math::vec3(1.0-0.1*i, 0.0, 0.1*i));
        cube->setParentAttachmentPoint(math::vec3(0.0, 1.0, 0.0), math::vec3(0.0, 0.0, 0.0));
        cube->addAttachmentPoint(math::vec3(1.0, 0.0, 0.0), math::vec3(0.0, 0.0, 0.0));
        cube->addAttachmentPoint(math::vec3(0.0, -1.0, 0.0), math::vec3(0.0, 0.0, 0.0));
        cube->addAttachmentPoint(math::vec3(1.0, 0.0, 1.0), math::vec3(0.0, 0.0, 0.0));
        m_parts.push_back(std::move(std::unique_ptr<BasePart>(cube)));
    }

    m_collision_shapes.push_back(std::move(cube_shape_ground));
    m_collision_shapes.push_back(std::move(cube_shape));
    m_collision_shapes.push_back(std::move(sphere_shape));
    m_collision_shapes.push_back(std::move(cube3m));
    m_collision_shapes.push_back(std::move(cylinder_shape));
}


void App::run(){

    /* THIS SHOULD BE MOVED SOMEWHERE ELSE, THIS IS JUST FOR TESTING*/
    // font atlas = nullptr because I don't have text there yet
    m_editor_gui.reset(new EditorGUI(m_window_handler.get(), nullptr, m_render_context->getShader(SHADER_GUI), m_render_context.get()));
    m_render_context->m_gui = m_editor_gui.get();
    /* THIS SHOULD BE MOVED SOMEWHERE ELSE, THIS IS JUST FOR TESTING*/

    std::chrono::steady_clock::time_point loop_start_load;
    std::chrono::steady_clock::time_point previous_loop_start_load = std::chrono::steady_clock::now();;
    std::chrono::steady_clock::time_point loop_end_load;
    double delta_t = (1. / 60.) * 1000000., accumulated_load = 0.0, accumulated_sleep = 0.0, average_load = 0.0, average_sleep = 0.0;
    int ticks_since_last_update = 0;

    m_bt_wrapper->startSimulation(1.f / 60.f, 2);
    m_render_context->start();
    while (!glfwWindowShouldClose(m_window_handler->getWindow())){
        loop_start_load = std::chrono::steady_clock::now();

        // process queues
        for(uint i=0; i < m_set_motion_state_queue.size(); i++){
            struct set_motion_state_msg& msg = m_set_motion_state_queue.at(i);
            msg.object->setMotionState(msg.origin, msg.initial_rotation);
        }
        m_set_motion_state_queue.clear();

        for(uint i=0; i < m_add_constraint_queue.size(); i++){
            struct add_contraint_msg& msg = m_add_constraint_queue.at(i);
            msg.part->setParentConstraint(msg.constraint_uptr);
        }
        m_add_constraint_queue.clear();

        {  //wake up physics thread
            std::unique_lock<std::mutex> lck2(m_thread_monitor.mtx_start);
            m_thread_monitor.worker_start = true;
            m_thread_monitor.cv_start.notify_all();
        }

        m_input->update();
        m_window_handler->update();
        m_camera->update();
        m_frustum->extractPlanes(m_camera->getViewMatrix(), m_camera->getProjMatrix(), false);

        logic();

        m_render_context->setDebugOverlayTimes(m_bt_wrapper->getAverageLoadTime(), average_load, average_sleep);
        
        m_elapsed_time += loop_start_load - previous_loop_start_load;
        previous_loop_start_load = loop_start_load;
        
        if(ticks_since_last_update == 60){
            ticks_since_last_update = 0;
            average_load = accumulated_load / 60000.0;
            average_sleep = accumulated_sleep / 60000.0;
            accumulated_load = 0;
            accumulated_sleep = 0;
            /*std::cout << std::setfill('0') << std::setw(2) << int(m_elapsed_time.count() / 1e12) / 60*60 << ":" 
                      << std::setfill('0') << std::setw(2) << (int(m_elapsed_time.count() / 1e6) / 60) % 60 << ":" 
                      << std::setfill('0') << std::setw(2) << int(m_elapsed_time.count() / 1e6) % 60 << std::endl;*/
        }
        ticks_since_last_update++;

        { // wait for physics thread
            std::unique_lock<std::mutex> lck(m_thread_monitor.mtx_end);
            while(!m_thread_monitor.worker_ended){
                m_thread_monitor.cv_end.wait(lck);
            }
            m_thread_monitor.worker_ended = false;
        }

        // load ends here

        loop_end_load = std::chrono::steady_clock::now();
        std::chrono::duration<double, std::micro> load_time = loop_end_load - loop_start_load;
        accumulated_load += load_time.count();
        accumulated_sleep += delta_t - load_time.count();

        if(load_time.count() < delta_t){
            std::chrono::duration<double, std::micro> delta_ms(delta_t - load_time.count());
            std::this_thread::sleep_for(delta_ms);
        }
    }
    m_bt_wrapper->stopSimulation();
    m_render_context->stop();
    m_window_handler->terminate();
}


void App::logic(){
    // temporal method with the game logic
    //bool first_lmb_click = false;

    // mouse pick test
    if(m_picked_obj){
        math::vec3 ray_start_world, ray_end_world;
        btVector3 ray_end_world_btv3;
        BasePart* part;
        
        m_camera->castRayMousePos(25.f, ray_start_world, ray_end_world);
        ray_end_world_btv3 = btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]);

        // some spaghetti code testing the attachments
        // disorganized as fuck
        part = dynamic_cast<BasePart*>(m_picked_obj);
        if(part){
            const math::mat4 proj_mat = m_camera->getProjMatrix();
            const math::mat4 view_mat = m_camera->getViewMatrix();

            double mousey, mousex;
            int w, h;
            m_input->getMousePos(mousex, mousey);
            m_window_handler->getFramebufferSize(w, h);
            mousey = ((mousey / h) * 2 - 1) * -1;
            mousex = (mousex / w) * 2 - 1;

            float closest_dist = 99999999999.9;
            math::vec4 closest_att_point_world;
            math::vec3 closest_att_point;
            const BasePart* closest;

            for(uint i=0; i<m_parts.size(); i++){
                if(part == m_parts.at(i).get()){
                    continue;
                }

                const std::vector<struct attachment_point>* att_points = m_parts.at(i)->getAttachmentPoints();

                for(uint j=0; j<att_points->size(); j++){
                    const math::vec3 att_point = att_points->at(j).point;
                    math::mat4 att_transform = math::translate(math::identity_mat4(), att_point);
                    att_transform = m_parts.at(i)->getRigidBodyTransformSingle() * att_transform;
                    math::vec4 att_point_loc_world = math::vec4(att_transform.m[12], att_transform.m[13], att_transform.m[14], 1.0);
                    math::vec4 att_point_loc_screen;

                    att_point_loc_screen = proj_mat * view_mat * att_point_loc_world;
                    att_point_loc_screen = att_point_loc_screen / att_point_loc_screen.v[3];

                    float distance = math::distance(math::vec2(mousex, mousey),
                                                    math::vec2(att_point_loc_screen.v[0], att_point_loc_screen.v[1]));
                    
                    if(distance < closest_dist){
                        closest_dist = distance;
                        closest_att_point_world = att_point_loc_world;
                        closest_att_point = att_point;
                        closest = m_parts.at(i).get();
                    }
                }
            }
            btTransform transform_original;
            btQuaternion rotation;
            m_picked_obj->m_body->getMotionState()->getWorldTransform(transform_original);
            rotation = transform_original.getRotation();

            if(closest_dist < 0.05){
                btTransform transform_final;
                btVector3 btv3_child_att(part->getParentAttachmentPoint()->point.v[0],
                                         part->getParentAttachmentPoint()->point.v[1],
                                         part->getParentAttachmentPoint()->point.v[2]);
                btVector3 btv3_closest_att_world(closest_att_point_world.v[0],
                                                 closest_att_point_world.v[1],
                                                 closest_att_point_world.v[2]);

                transform_final = btTransform(btQuaternion::getIdentity(), -btv3_child_att);

                btTransform object_R = btTransform(rotation, btVector3(0.0, 0.0, 0.0));
                btTransform object_T = btTransform(btQuaternion::getIdentity(), btv3_closest_att_world);

                transform_final = object_T * object_R * transform_final; // rotated and traslated attachment point (world)

                m_set_motion_state_queue.emplace_back(set_motion_state_msg{m_picked_obj, transform_final.getOrigin(), transform_final.getRotation()}); // thread safe :)))

                if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS){ // user has decided to attach the object to the parent
                    btTransform parent_transform, frame_child;

                    closest->m_body->getMotionState()->getWorldTransform(parent_transform);
                    frame_child = btTransform(transform_final.inverse() * parent_transform);

                    btGeneric6DofConstraint* constraint = new btGeneric6DofConstraint(*closest->m_body, *m_picked_obj->m_body, 
                                                                                      btTransform::getIdentity(), frame_child, false);

                    constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 0);
                    constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 1);
                    constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 2);
                    constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 3);
                    constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 4);
                    constraint->setParam(BT_CONSTRAINT_STOP_CFM, 0.f, 5);

                    constraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 0);
                    constraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 1);
                    constraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 2);
                    constraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 3);
                    constraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 4);
                    constraint->setParam(BT_CONSTRAINT_STOP_ERP, 0.8f, 5);

                    constraint->setOverrideNumSolverIterations(100); // improved stiffness??
                    // also add 2 constraints??

                    btVector3 limits = btVector3(0, 0, 0);
                    constraint->setLinearLowerLimit(limits);
                    constraint->setLinearUpperLimit(limits);
                    constraint->setAngularLowerLimit(limits);
                    constraint->setAngularUpperLimit(limits);
  
                    m_add_constraint_queue.emplace_back(add_contraint_msg{part, std::unique_ptr<btTypedConstraint>(constraint)});
                }
            }
            else{
                if(m_input->keyboardPressed()){
                    btQuaternion rotation2(0.0, 0.0, 0.0); // allows the user to rotate the part
                    if(m_input->pressed_keys[GLFW_KEY_U] & INPUT_KEY_DOWN){
                        rotation2.setEuler(M_PI/2.0, 0., 0.);
                    }
                    else if(m_input->pressed_keys[GLFW_KEY_O] & INPUT_KEY_DOWN){
                        rotation2.setEuler(-M_PI/2.0, 0., 0.);
                    }
                    else if(m_input->pressed_keys[GLFW_KEY_I] & INPUT_KEY_DOWN){
                        rotation2.setEuler(0., M_PI/2.0, 0.);
                    }
                    else if(m_input->pressed_keys[GLFW_KEY_K] & INPUT_KEY_DOWN){
                        rotation2.setEuler(0., -M_PI/2.0, 0.);
                    }
                    else if(m_input->pressed_keys[GLFW_KEY_J] & INPUT_KEY_DOWN){
                        rotation2.setEuler(0., 0., M_PI/2.0);
                    }
                    else if(m_input->pressed_keys[GLFW_KEY_L] & INPUT_KEY_DOWN){
                        rotation2.setEuler(0., 0., -M_PI/2.0);
                    }
                    rotation = rotation * rotation2;
                }
                
                btVector3 origin(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]);
                m_set_motion_state_queue.emplace_back(set_motion_state_msg{m_picked_obj, origin, rotation});
            }
        }
        else{
            btTransform transform;
            btVector3 origin(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]);
            m_picked_obj->m_body->getMotionState()->getWorldTransform(transform);
            m_set_motion_state_queue.emplace_back(set_motion_state_msg{m_picked_obj, origin, transform.getRotation()});
        }

        if(m_physics_pause)
            m_bt_wrapper->updateCollisionWorldSingleAABB(m_picked_obj->m_body.get()); // not thread safe
    }

    if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS){
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

    if(m_input->pressed_keys[GLFW_KEY_P] == INPUT_KEY_DOWN){
        m_physics_pause = !m_physics_pause;
        m_bt_wrapper->pauseSimulation(m_physics_pause);
    }
}

