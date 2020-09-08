#include "App.hpp"


App::App() : BaseApp(){
    init();
}


App::App(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


void App::init(){
    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_physics_pause = true;
    m_picked_obj = nullptr;
    m_clear_scene = false;
    m_buffers.last_updated = none;
    m_vessel_id = 0;
    m_delete_current = false;

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);

    m_editor_gui.reset(new EditorGUI(m_def_font_atlas.get(), m_render_context.get(), m_input.get()));
    m_editor_gui->setMasterPartList(&m_asset_manager->m_master_parts);
    m_render_context->setEditorGUI(m_editor_gui.get());
}


App::~App(){
}


void App::run(){
    std::chrono::steady_clock::time_point loop_start_load;
    std::chrono::steady_clock::time_point previous_loop_start_load = std::chrono::steady_clock::now();;
    std::chrono::steady_clock::time_point loop_end_load;
    double delta_t = (1. / 60.) * 1000000., accumulated_load = 0.0, accumulated_sleep = 0.0, average_load = 0.0, average_sleep = 0.0;
    int ticks_since_last_update = 0;

    m_render_context->setEditorMode(GUI_MODE_EDITOR);
    m_bt_wrapper->startSimulation(1.f / 60.f, 2);
    m_render_context->start();
    while (!glfwWindowShouldClose(m_window_handler->getWindow())){
        loop_start_load = std::chrono::steady_clock::now();

        m_asset_manager->processCommandBuffers(m_physics_pause);
        if(m_clear_scene){
            clearScene();
        }
        if(m_delete_current){
            deleteCurrent();
        }

        m_input->update();
        m_window_handler->update();
        m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
        m_player->update();
        m_asset_manager->updateBuffers();

        {  //wake up physics thread
            std::unique_lock<std::mutex> lck2(m_thread_monitor.mtx_start);
            m_thread_monitor.worker_start = true;
            m_thread_monitor.cv_start.notify_all();
        }

        m_gui_action = m_editor_gui->update();

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


void App::getClosestAtt(float& closest_dist, math::vec4& closest_att_point_world, BasePart*& closest, BasePart* part){
    const math::mat4 proj_mat = m_camera->getProjMatrix();
    const math::mat4 view_mat = m_camera->getViewMatrix();
    double mousey, mousex;
    int w, h;

    m_input->getMousePos(mousex, mousey);
    m_window_handler->getFramebufferSize(w, h);
    mousey = ((mousey / h) * 2 - 1) * -1;
    mousex = (mousex / w) * 2 - 1;

    std::vector<BasePart*>* vessel_parts = m_asset_manager->m_editor_vessels.at(m_vessel_id)->getParts();
    for(uint i=0; i<vessel_parts->size(); i++){ // get closest att point to the mouse cursor
        if(part == vessel_parts->at(i)){
            continue;
        }

        const std::vector<struct attachment_point>* att_points = vessel_parts->at(i)->getAttachmentPoints();

        for(uint j=0; j<att_points->size(); j++){
            const math::vec3 att_point = att_points->at(j).point;
            math::mat4 att_transform = math::translate(math::identity_mat4(), att_point);
            att_transform = vessel_parts->at(i)->getRigidBodyTransformSingle() * att_transform;
            math::vec4 att_point_loc_world = math::vec4(att_transform.m[12], att_transform.m[13], att_transform.m[14], 1.0);
            math::vec4 att_point_loc_screen;

            att_point_loc_screen = proj_mat * view_mat * att_point_loc_world;
            att_point_loc_screen = att_point_loc_screen / att_point_loc_screen.v[3];

            float distance = math::distance(math::vec2(mousex, mousey),
                                            math::vec2(att_point_loc_screen.v[0], att_point_loc_screen.v[1]));
            
            if(distance < closest_dist){
                closest_dist = distance;
                closest_att_point_world = att_point_loc_world;
                closest = vessel_parts->at(i);
            }
        }
    }
}


void App::getUserRotation(btQuaternion& rotation, const btQuaternion& current_rotation){
    if(m_input->keyboardPressed() && !m_render_context->imGuiWantCaptureKeyboard()){
        if(m_input->pressed_keys[GLFW_KEY_U] & INPUT_KEY_DOWN){
            rotation.setEuler(M_PI/2.0, 0., 0.);
        }
        else if(m_input->pressed_keys[GLFW_KEY_O] & INPUT_KEY_DOWN){
            rotation.setEuler(-M_PI/2.0, 0., 0.);
        }
        else if(m_input->pressed_keys[GLFW_KEY_I] & INPUT_KEY_DOWN){
            rotation.setEuler(0., M_PI/2.0, 0.);
        }
        else if(m_input->pressed_keys[GLFW_KEY_K] & INPUT_KEY_DOWN){
            rotation.setEuler(0., -M_PI/2.0, 0.);
        }
        else if(m_input->pressed_keys[GLFW_KEY_J] & INPUT_KEY_DOWN){
            rotation.setEuler(0., 0., M_PI/2.0);
        }
        else if(m_input->pressed_keys[GLFW_KEY_L] & INPUT_KEY_DOWN){
            rotation.setEuler(0., 0., -M_PI/2.0);
        }
        else if(m_input->pressed_keys[GLFW_KEY_R] & INPUT_KEY_DOWN){
            rotation = current_rotation.inverse();
        }
    }
}


void App::placeSubTree(float closest_dist, math::vec4& closest_att_point_world, BasePart* closest, BasePart* part){
    btTransform transform_original;
    btQuaternion rotation;
    m_picked_obj->m_body->getMotionState()->getWorldTransform(transform_original);
    rotation = transform_original.getRotation();

    if(closest_dist < 0.05 && !part->isRoot() && part->hasParentAttPoint()){ // magnet
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

        btVector3 disp = transform_final.getOrigin() - transform_original.getOrigin();
        part->updateSubTreeMotionState(m_asset_manager->m_set_motion_state_buffer, disp, transform_original.getOrigin(), 
                                       part->m_user_rotation * rotation.inverse());

        if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS && !m_render_context->imGuiWantCaptureMouse()){
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

            m_asset_manager->m_add_constraint_buffer.emplace_back(add_contraint_msg{part, std::unique_ptr<btTypedConstraint>(constraint)});
            BasePart* parent = part->getParent(); // temove this??
            if(parent){ // sanity check
                parent->removeChild(part);
            }

            std::shared_ptr<BasePart> part_sptr = std::dynamic_pointer_cast<BasePart>(part->getSharedPtr());
            m_asset_manager->m_editor_vessels.at(closest->getVessel()->getId())->addChildById(part_sptr, closest->getUniqueId());

            m_asset_manager->m_editor_subtrees.erase(part->getUniqueId());
        }
    }
    else{
        dmath::vec3 ray_start_world, ray_end_world;
        btQuaternion user_rotation(0.0, 0.0, 0.0);
        m_camera->castRayMousePos(1000.f, ray_start_world, ray_end_world);
        btCollisionWorld::ClosestRayResultCallback ray_callback(btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                                                btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
        ray_callback.m_collisionFilterGroup = CG_RAY_EDITOR_RADIAL;

        Object* obj = m_bt_wrapper->testRay(ray_callback, 
                                            btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));

        // get+update user totation
        getUserRotation(user_rotation, rotation);
        part->m_user_rotation = part->m_user_rotation * user_rotation;

        if(obj && !part->isRoot() && part->hasFreeAttPoint()){ // free attaching
            btVector3 btv3_child_att;
            btQuaternion align_rotation;
            math::versor align_rot_q;
            math::mat3 align_rot;

            math::vec3 child_att = part->getFreeAttachmentPoint()->point;
            math::vec3 child_att_orientation(0.0, 0.0, 0.0);
            child_att_orientation = child_att_orientation - part->getFreeAttachmentPoint()->orientation; //??DS?D?SADsjfk
            math::vec3 surface_normal = math::vec3(ray_callback.m_hitNormalWorld.getX(),
                                                   ray_callback.m_hitNormalWorld.getY(),
                                                   ray_callback.m_hitNormalWorld.getZ());

            if(dot(surface_normal, child_att_orientation) == -1){
                math::mat4 aux;
                align_rot = rotation_align(math::arb_perpendicular(child_att_orientation), child_att_orientation);
                child_att = align_rot * child_att;
                align_rot = rotation_align(surface_normal, math::arb_perpendicular(child_att_orientation));
                child_att = align_rot * child_att;
            }
            else{
                align_rot = rotation_align(surface_normal, child_att_orientation);
            }

            align_rot_q = math::from_mat3(align_rot);
            align_rotation = btQuaternion(align_rot_q.q[0], align_rot_q.q[1], align_rot_q.q[2], align_rot_q.q[3]);
            btv3_child_att = btVector3(child_att.v[0], child_att.v[1], child_att.v[2]);

            btTransform transform_final = btTransform(btQuaternion::getIdentity(), -btv3_child_att);
            btTransform object_R = btTransform(rotation, btVector3(0.0, 0.0, 0.0));
            btTransform object_T = btTransform(btQuaternion::getIdentity(), ray_callback.m_hitPointWorld);

            transform_final = object_T * object_R * transform_final;

            btVector3 disp = transform_final.getOrigin() - transform_original.getOrigin();
            part->updateSubTreeMotionState(m_asset_manager->m_set_motion_state_buffer, disp, transform_original.getOrigin(),
                                           align_rotation * part->m_user_rotation * rotation.inverse());

            if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS && !m_render_context->imGuiWantCaptureMouse()){
                btTransform parent_transform;
                BasePart* parent = static_cast<BasePart*>(obj);

                parent->m_body->getMotionState()->getWorldTransform(parent_transform);

                btGeneric6DofConstraint* constraint = new btGeneric6DofConstraint(*parent->m_body, *part->m_body, btTransform::getIdentity(),
                                                                                  transform_final.inverse() * parent_transform, false);

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

                constraint->setOverrideNumSolverIterations(100);

                btVector3 limits = btVector3(0, 0, 0);
                constraint->setLinearLowerLimit(limits);
                constraint->setLinearUpperLimit(limits);
                constraint->setAngularLowerLimit(limits);
                constraint->setAngularUpperLimit(limits);

                m_asset_manager->m_add_constraint_buffer.emplace_back(add_contraint_msg{part, std::unique_ptr<btTypedConstraint>(constraint)});

                std::shared_ptr<BasePart> part_sptr = std::dynamic_pointer_cast<BasePart>(part->getSharedPtr());
                m_asset_manager->m_editor_vessels.at(parent->getVessel()->getId())->addChildById(part_sptr, parent->getUniqueId());

                m_asset_manager->m_editor_subtrees.erase(part->getUniqueId());

            }
        }
        else{
            m_camera->castRayMousePos(10.f, ray_start_world, ray_end_world);
            btVector3 origin(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]);
            btVector3 disp = origin - transform_original.getOrigin();
            part->updateSubTreeMotionState(m_asset_manager->m_set_motion_state_buffer, disp, transform_original.getOrigin(),
                                           part->m_user_rotation * rotation.inverse());
        }
    }
}


void App::pickObject(){
    double mousey, mousex;
    dmath::vec3 ray_start_world, ray_end_world;
    Object* obj;

    m_input->getMousePos(mousex, mousey);
    m_camera->castRayMousePos(1000.f, ray_start_world, ray_end_world);

    btCollisionWorld::ClosestRayResultCallback ray_callback(btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                                            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
    ray_callback.m_collisionFilterGroup = CG_RAY_EDITOR_SELECT;

    obj = m_bt_wrapper->testRay(ray_callback, 
                                btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
    if(obj){
        BasePart* part = static_cast<BasePart*>(obj);

        m_picked_obj = obj;
        if(part->getVessel() == nullptr){
            while(part->getParent() != nullptr){
               part = part->getParent();
            }
            m_picked_obj = part;
        }
        else{ // we are certainly detaching a part from the vessel
            if(!part->isRoot()){ // ignore root
                m_asset_manager->m_remove_part_constraint_buffer.emplace_back(part);
                m_asset_manager->m_editor_subtrees.insert({part->getUniqueId(), m_asset_manager->m_editor_vessels.at(part->getVessel()->getId())->removeChild(part)});
            }
        }
    }
}


void App::logic(){
    if(m_picked_obj && m_gui_action != EDITOR_ACTION_DELETE){
        BasePart* part = static_cast<BasePart*>(m_picked_obj);

        float closest_dist = 99999999999.9;;
        math::vec4 closest_att_point_world;
        BasePart* closest = nullptr;

        if(m_vessel_id != 0){
            getClosestAtt(closest_dist, closest_att_point_world, closest, part);
        }
        placeSubTree(closest_dist, closest_att_point_world, closest, part);

        if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS && !m_render_context->imGuiWantCaptureMouse()){
            m_picked_obj->activate(true);
            m_picked_obj = nullptr;
        }
    }
    else{ // if not picked object
        if(!m_gui_action && m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS &&
            m_physics_pause && !m_render_context->imGuiWantCaptureMouse()){ // scene has the focus
            pickObject();
        }
    }

    // GUI processing
    if(m_gui_action == EDITOR_ACTION_OBJECT_PICK && m_physics_pause){
        const std::unique_ptr<BasePart>* editor_picked_object = m_editor_gui->getPickedObject();
        const BasePart* part_ptr = editor_picked_object->get(); // trickery
        std::shared_ptr<BasePart> part = std::make_shared<BasePart>(*part_ptr);

        if(m_picked_obj){ // if the user has an scene object picked just leave it "there"
            m_picked_obj->activate(true);
            m_picked_obj = nullptr;
        }

        if(!m_vessel_id){ // set the vessel root
            m_asset_manager->m_add_body_buffer.emplace_back(add_body_msg{part.get(), btVector3(0.0, 60.0, 0.0),
                                                            btVector3(0.0, 0.0, 0.0), btQuaternion::getIdentity()});

            std::shared_ptr<Vessel> vessel = std::make_shared<Vessel>(part);
            m_vessel_id = vessel->getId();
            m_asset_manager->m_editor_vessels.insert({m_vessel_id, vessel});
            part->setCollisionFilters(part->getCollisionFilters() | CG_RAY_EDITOR_RADIAL);
        }
        else{
            dmath::vec3 ray_start_world, ray_end_world;
            m_camera->castRayMousePos(10.f, ray_start_world, ray_end_world);
            m_asset_manager->m_add_body_buffer.emplace_back(add_body_msg{part.get(), btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]),
                                                            btVector3(0.0, 0.0, 0.0), btQuaternion::getIdentity()});

            m_asset_manager->m_editor_subtrees.insert({part->getUniqueId(), part});
            m_picked_obj = part.get();
        }
    }

    if(m_gui_action == EDITOR_ACTION_DELETE && m_physics_pause){
        m_delete_current = true;
    }

    // other input
    if(m_input->pressed_keys[GLFW_KEY_P] == INPUT_KEY_DOWN && !m_render_context->imGuiWantCaptureKeyboard()){
        m_physics_pause = !m_physics_pause;
        m_bt_wrapper->pauseSimulation(m_physics_pause);
        if(m_picked_obj){
            m_picked_obj->activate(true);
            m_picked_obj = nullptr;
        }
    }

    if(m_input->pressed_keys[GLFW_KEY_F12] == INPUT_KEY_DOWN && !m_render_context->imGuiWantCaptureKeyboard()){
        m_render_context->toggleDebugOverlay();
    }

    if(m_input->pressed_keys[GLFW_KEY_F] == INPUT_KEY_DOWN && !m_render_context->imGuiWantCaptureKeyboard()){
        m_clear_scene = true;
    }

    if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2] & INPUT_MBUTTON_RELEASE && !m_render_context->imGuiWantCaptureMouse() && !m_gui_action){
        onLeftMouseButton();
    }
}


void App::clearScene(){
    m_asset_manager->clearSceneEditor();
    m_vessel_id = 0;
    m_clear_scene = false;
    m_picked_obj = nullptr;
    std::cout << "Scene cleared" << std::endl;
}


void App::onLeftMouseButton(){
    dmath::vec3 ray_start_world, ray_end_world;
    Object* obj;
    BasePart* part;

    m_camera->castRayMousePos(1000.f, ray_start_world, ray_end_world);

    btCollisionWorld::ClosestRayResultCallback ray_callback(btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                                            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
    ray_callback.m_collisionFilterGroup = CG_RAY_EDITOR_SELECT;

    obj = m_bt_wrapper->testRay(ray_callback, 
                                btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));

    if(obj){
        part = static_cast<BasePart*>(obj);
        if(m_physics_pause){
            part->onEditorRightMouseButton();
        }
        else{
            part->onSimulationRightMouseButton();
        }
        
    }
}


void App::deleteCurrent(){
    /*In the future a command buffer should be used to delete multiple subtrees/vessels*/
    m_delete_current = false;

    if(!m_picked_obj){
        return;
    }

    m_asset_manager->deleteObjectEditor(static_cast<BasePart*>(m_picked_obj), m_vessel_id);
    m_picked_obj = nullptr;
}

