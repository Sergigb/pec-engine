#include <iostream>
#include <chrono>
#include <vector>

#include "GameEditor.hpp"
#include "../core/RenderContext.hpp"
#include "../core/AssetManager.hpp"
#include "../core/Physics.hpp"
#include "../core/WindowHandler.hpp"
#include "../core/Input.hpp"
#include "../core/Frustum.hpp"
#include "../core/Camera.hpp"
#include "../core/Player.hpp"
#include "../core/log.hpp"
#include "../core/multithreading.hpp"
#include "../core/BaseApp.hpp"
#include "../core/timing.hpp"
#include "../assets/Vessel.hpp"
#include "../assets/BasePart.hpp"
#include "../GUI/FontAtlas.hpp"
#include "../GUI/editor/EditorGUI.hpp"
#include "../GUI/DebugOverlay.hpp"
#include "../renderers/EditorRenderer.hpp"


#define MAX_SYMMETRY_SIDES 8
#define SIDE_ANGLE_STEP double(M_PI / MAX_SYMMETRY_SIDES)


bool lmb_focused_press = false;


GameEditor::GameEditor(BaseApp* app, FontAtlas* font_atlas){
    m_app = app;
    m_input = m_app->getInput();
    m_camera = m_app->getCamera();
    m_window_handler = m_app->getWindowHandler();
    m_frustum = m_app->getFrustum();
    m_render_context = m_app->getRenderContext();
    m_physics = m_app->getPhysics();
    m_asset_manager = m_app->getAssetManager();
    m_player = m_app->getPlayer();

    m_thread_monitor= m_app->getThreadMonitor();

    m_def_font_atlas = font_atlas;

    m_editor_gui.reset(new EditorGUI(m_app, m_def_font_atlas));
    m_editor_gui->setMasterPartList(&m_asset_manager->m_master_parts);
    m_render_context->setGUI(m_editor_gui.get(), GUI_MODE_EDITOR);
    m_renderer.reset(new EditorRenderer(m_app));
    m_app->getRenderContext()->setRenderer(m_renderer.get(), RENDER_EDITOR);

    init();
}


void GameEditor::init(){
    m_render_context->setLightPosition(math::vec3(500.0, 1000.0, 0.0));

    m_physics_pause = true;
    m_picked_obj = nullptr;
    m_clear_scene = false;
    m_vessel_id = 0;
    m_delete_current = false;
    m_symmetry_sides = 1;
    m_radial_align = true;
    m_exit_editor = false;
    m_highlight_part = nullptr;
    m_exit_code = EXIT_MAIN_MENU;
}


GameEditor::~GameEditor(){
}


void GameEditor::synchPreStep(){
    m_asset_manager->processCommandBuffers(m_physics_pause);
    if(m_asset_manager->m_editor_vessel.get())
        m_asset_manager->m_editor_vessel->updateCoM();

    if(m_clear_scene){
        clearScene();
    }
    if(m_delete_current){
        deleteCurrent();
    }

    m_input->update();
    m_window_handler->update();
    m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
}


void GameEditor::synchPostStep(){
    const Vessel* selected_vessel = m_player->getVessel();
    if(!selected_vessel){
        m_camera->freeCameraUpdate();
    }
    else{
        const btVector3& com = selected_vessel->getCoM();
        m_camera->setCameraPosition(dmath::vec3(com.getX(), com.getY(), com.getZ()));
        m_camera->orbitalCameraUpdate();
    }
    m_asset_manager->updateBuffers();
}


void GameEditor::wakePhysics(){
    std::unique_lock<std::mutex> lck2(m_thread_monitor->mtx_start);
    m_thread_monitor->worker_start = true;
    m_thread_monitor->cv_start.notify_all();
}


void GameEditor::waitPhysics(){
    std::unique_lock<std::mutex> lck(m_thread_monitor->mtx_end);
    while(!m_thread_monitor->worker_ended){
        m_thread_monitor->cv_end.wait(lck);
    }
    m_thread_monitor->worker_ended = false;
}


int GameEditor::start(){
    double delta_t_ms = (1. / 60.) * 1000000.;
    logic_timing timing;
    timing.delta_t = delta_t_ms;

    m_app->setGUIMode(GUI_MODE_EDITOR);
    m_app->setRenderState(RENDER_EDITOR);
    m_player->setBehaviour(PLAYER_BEHAVIOUR_EDITOR);

    while(!m_exit_editor){
        timing.register_tp(TP_LOGIC_START);

        synchPreStep();
        wakePhysics();
        logic();

        m_render_context->getDebugOverlay()->setLogicTimes(timing);

        waitPhysics();
        synchPostStep();

        timing.register_tp(TP_LOGIC_END);
        timing.update();

        if(timing.current_sleep > 0.0){
            std::this_thread::sleep_for(duration(timing.current_sleep));
        }
    }
    clearSymmetrySubtrees();

    return m_exit_code;
}


void GameEditor::getClosestAtt(float& closest_dist, math::vec4& closest_att_point_world, BasePart*& closest, BasePart* part){
    const math::mat4 proj_mat = m_camera->getProjMatrix();
    const math::mat4 view_mat = m_camera->getViewMatrix();
    double mousey, mousex;
    int w, h;

    m_input->getMousePos(mousex, mousey);
    m_window_handler->getFramebufferSize(w, h);
    mousey = ((mousey / h) * 2 - 1) * -1;
    mousex = (mousex / w) * 2 - 1;

    const std::vector<BasePart*>& vessel_parts = m_asset_manager->m_editor_vessel->getParts();
    for(uint i=0; i<vessel_parts.size(); i++){ // get closest att point to the mouse cursor
        if(part == vessel_parts.at(i)){
            continue;
        }

        const std::vector<struct attachment_point>& att_points = vessel_parts.at(i)->getAttachmentPoints();

        for(uint j=0; j<att_points.size(); j++){
            const math::vec3& att_point = att_points.at(j).point;
            math::mat4 att_transform = math::translate(math::identity_mat4(), att_point);
            att_transform = vessel_parts.at(i)->getRigidBodyTransformSingle() * att_transform;
            math::vec4 att_point_loc_world = math::vec4(att_transform.m[12], att_transform.m[13], att_transform.m[14], 1.0);
            math::vec4 att_point_loc_screen;

            att_point_loc_screen = proj_mat * view_mat * att_point_loc_world;
            att_point_loc_screen = att_point_loc_screen / att_point_loc_screen.v[3];

            float distance = math::distance(math::vec2(mousex, mousey),
                                            math::vec2(att_point_loc_screen.v[0], att_point_loc_screen.v[1]));
            
            if(distance < closest_dist){
                closest_dist = distance;
                closest_att_point_world = att_point_loc_world;
                closest = vessel_parts.at(i);
            }
        }
    }
}


void GameEditor::getUserRotation(btQuaternion& rotation, const btQuaternion& current_rotation){
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


void GameEditor::createConstraint(BasePart* part, BasePart* parent, btTransform frame){
    btTransform parent_transform;
    parent->m_body->getMotionState()->getWorldTransform(parent_transform);

    btGeneric6DofConstraint* constraint = new btGeneric6DofConstraint(*parent->m_body, *part->m_body, 
                                                                      btTransform::getIdentity(), frame, false);

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

    std::unique_ptr<btTypedConstraint> constraint_sptr(constraint);

    m_asset_manager->addConstraint(part, std::move(constraint_sptr));
}


void GameEditor::clearSymmetrySubtrees(){
    if(m_asset_manager->m_symmetry_subtrees.size()){
        for(uint i=0; i < m_asset_manager->m_symmetry_subtrees.size(); i++){
            m_asset_manager->m_symmetry_subtrees.at(i)->clearSubTreeCloneData();
            m_asset_manager->deleteSubtree(std::move(m_asset_manager->m_symmetry_subtrees.at(i)));
        }
        m_asset_manager->m_symmetry_subtrees.clear();
    }
}


void GameEditor::createSymmetrySubtrees(){
    if(m_asset_manager->m_symmetry_subtrees.size() != m_symmetry_sides - 1 && m_symmetry_sides > 1){
        BasePart* part = static_cast<BasePart*>(m_picked_obj);
        clearSymmetrySubtrees();

        for(uint i=0; i < m_symmetry_sides - 1; i++){
            std::shared_ptr<BasePart> clone;
            part->cloneSubTree(clone, true, true);
            m_asset_manager->m_symmetry_subtrees.emplace_back(clone);
        }
    }
    else if(m_symmetry_sides == 1){
        clearSymmetrySubtrees();
    }
}


void GameEditor::hitPointAlign(btVector3& hit_point_world, btVector3& hit_normal_world, btTransform& parent_transform){
    btQuaternion rotation;
    btTransform trans;
    btVector3 hit_point_local = parent_transform.inverse() * hit_point_world;
    btVector3 hit_normal_local = btTransform(parent_transform.getRotation().inverse()) * hit_normal_world;
    
    double phi = std::atan(hit_point_local.getZ() / hit_point_local.getX());
    double angle = phi - (std::round(phi / SIDE_ANGLE_STEP) * SIDE_ANGLE_STEP);

    rotation.setEuler(angle, 0.0, 0.0);
    trans = btTransform(rotation);

    hit_point_local = trans * hit_point_local;
    hit_normal_local = trans * hit_normal_local;

    hit_point_world = parent_transform * hit_point_local;
    hit_normal_world = btTransform(parent_transform.getRotation()) * hit_normal_local;
}


void GameEditor::placeClonedSubtreesOnClones(BasePart* closest, btTransform& transform_final, std::vector<BasePart*>& clone_to){
    btVector3 part_trans_local;
    btTransform closest_transform;

    if(m_asset_manager->m_symmetry_subtrees.size() != clone_to.size()){
        std::cerr << "GameEditor::placeClonedSubtreesOnClones: The number of symmetric subtrees (" << m_asset_manager->m_symmetry_subtrees.size()
                  << ") does not match the number of parts to clone to (" << clone_to.size() << ")" << std::endl;

        log("GameEditor::placeClonedSubtreesOnClones: The number of symmetric subtrees (", m_asset_manager->m_symmetry_subtrees.size(),
            ") does not match the number of parts to clone to (", clone_to.size(), ")");
        return;
    }

    closest->m_body->getMotionState()->getWorldTransform(closest_transform);
    part_trans_local = transform_final.getOrigin() - closest_transform.getOrigin();

    for(uint i=0; i < m_asset_manager->m_symmetry_subtrees.size(); i++){
        std::shared_ptr<BasePart>& current = m_asset_manager->m_symmetry_subtrees.at(i);
        BasePart* current_parent = clone_to.at(i);
        if(current->m_body.get()){ // check if m_body is initialized, it is not during the first tick
            btTransform transform_original, transform_parent, transform_final_current;
            btVector3 current_trans_local, disp;
            btQuaternion rot_diff;

            current_parent->m_body->getMotionState()->getWorldTransform(transform_parent);
            current->m_body->getMotionState()->getWorldTransform(transform_original);

            rot_diff = transform_parent.getRotation() * closest_transform.getRotation().inverse();
            current_trans_local = transform_parent.getOrigin() + (btTransform(rot_diff) * part_trans_local);
            transform_final_current = btTransform(rot_diff * transform_final.getRotation(), current_trans_local);

            disp = transform_final_current.getOrigin() - transform_original.getOrigin();

            current->updateSubTreeMotionState(disp, transform_final_current.getOrigin(),
                                              transform_final_current.getRotation() * transform_original.getRotation().inverse());

            if(lmb_focused_press){
                createConstraint(current.get(), current_parent, transform_final_current.inverse() * transform_parent);
                m_asset_manager->m_editor_vessel->addChildById(std::move(current), current_parent->getUniqueId());
            }
        }
    }
    if(lmb_focused_press){
        m_asset_manager->m_symmetry_subtrees.clear();
    }
}


void rotate_y_deg(math::mat3 &m, float deg){ // the implementation form the maths file is for mat4, so I made this one here for now
    // convert to radians
    float rad = deg * ONE_DEG_IN_RAD;
    m.m[0] = cos( rad );
    m.m[6] = sin( rad );
    m.m[2] = -sin( rad );
    m.m[8] = cos( rad );
}


void GameEditor::placeSubTree(float closest_dist, math::vec4& closest_att_point_world, BasePart* closest, BasePart* part){
    btTransform transform_original;
    btQuaternion rotation;
    m_picked_obj->m_body->getMotionState()->getWorldTransform(transform_original);
    rotation = transform_original.getRotation();

    if(closest_dist < 0.05 && !part->isRoot() && part->hasParentAttPoint()){ // magnet
        btTransform transform_final;
        btVector3 btv3_child_att(part->getParentAttachmentPoint().point.v[0],
                                 part->getParentAttachmentPoint().point.v[1],
                                 part->getParentAttachmentPoint().point.v[2]);
        btVector3 btv3_closest_att_world(closest_att_point_world.v[0],
                                         closest_att_point_world.v[1],
                                         closest_att_point_world.v[2]);

        transform_final = btTransform(btQuaternion::getIdentity(), -btv3_child_att);

        btTransform object_T = btTransform(btQuaternion::getIdentity(), btv3_closest_att_world);
        transform_final = object_T * btTransform(part->m_user_rotation) * transform_final; // rotated and traslated attachment point (world)

        btVector3 disp = transform_final.getOrigin() - transform_original.getOrigin();
        part->updateSubTreeMotionState(disp, transform_original.getOrigin(),
                                       part->m_user_rotation * rotation.inverse());

        btTransform parent_transform;
        closest->m_body->getMotionState()->getWorldTransform(parent_transform);

        if(lmb_focused_press){
            createConstraint(part, closest, transform_final.inverse() * parent_transform);

            m_asset_manager->m_editor_vessel->addChildById(std::dynamic_pointer_cast<BasePart>(part->getSharedPtr()),
                                                           closest->getUniqueId());
            m_asset_manager->m_editor_subtrees.erase(part->getUniqueId());
        }

        if((closest->getClonedFrom() || closest->getClones().size()) && m_symmetry_sides > 1){  // place symmetric subtrees if necessary
            BasePart* cloned_from;  // part that "closest" was cloned from
            std::vector<BasePart*> clone_to;
            if(closest->getClonedFrom()){
                cloned_from = closest->getClonedFrom();

                for(uint i=0; i < cloned_from->getClones().size(); i++){
                    if(cloned_from->getClones().at(i) != closest){
                        clone_to.emplace_back(cloned_from->getClones().at(i));
                    }
                }
                clone_to.emplace_back(cloned_from);
            }
            else{
                cloned_from = closest;
                clone_to = closest->getClones();
            }

            if(m_symmetry_sides != cloned_from->getClones().size() + 1){
                m_symmetry_sides = cloned_from->getClones().size() + 1;
                m_editor_gui->setSymmetrySides(m_symmetry_sides);
            }
            createSymmetrySubtrees();
            placeClonedSubtreesOnClones(closest, transform_final, clone_to);
        }
        else{
            clearSymmetrySubtrees();
        }
    }
    else{
        dmath::vec3 ray_start_world, ray_end_world;
        btQuaternion user_rotation(0.0, 0.0, 0.0);
        m_camera->castRayMousePos(1000.f, ray_start_world, ray_end_world);
        btCollisionWorld::ClosestRayResultCallback ray_callback(btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                                                btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
        ray_callback.m_collisionFilterGroup = CG_RAY_EDITOR_RADIAL;

        Object* obj = m_physics->testRay(ray_callback, 
                                            btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));

        // get+update user totation
        getUserRotation(user_rotation, rotation);
        part->m_user_rotation = part->m_user_rotation * user_rotation;

        if(obj && !part->isRoot() && part->hasFreeAttPoint()){ // free attaching
            /*

            I'm changing alot from math:: to bullet math, I think I should make the attachments use bullet vectors to avoid this,
            and implement math::arb_perpendicular for bullet vectors. This is the todo list.

            */
            btVector3 btv3_child_att;
            btVector3 hit_normal_world = ray_callback.m_hitNormalWorld;
            btVector3 hit_point_world = ray_callback.m_hitPointWorld;
            btQuaternion align_rotation;
            math::versor align_rot_q;
            math::mat3 align_rot;

            btTransform p_transform;
            obj->m_body->getMotionState()->getWorldTransform(p_transform);
            btTransform object_iR(p_transform.getRotation().inverse(), btVector3(0.0, 0.0, 0.0));

            if(m_radial_align){
                hitPointAlign(hit_point_world, hit_normal_world, p_transform);
            }

            const math::vec3& child_att = part->getFreeAttachmentPoint().point;
            math::vec3 child_att_orientation(0.0, 0.0, 0.0);
            child_att_orientation = child_att_orientation - part->getFreeAttachmentPoint().orientation;

            // Invert the normal's rotation according to the parent's rotation, this way arb_perpendicular doesn't act funny when it's rotated
            hit_normal_world = object_iR * hit_normal_world;

            math::vec3 surface_normal = math::vec3(hit_normal_world.getX(),
                                                   hit_normal_world.getY(),
                                                   hit_normal_world.getZ());

            if(dot(surface_normal, child_att_orientation) == -1){
                align_rot = math::identity_mat3();
                rotate_y_deg(align_rot, 180.0f);
            }
            else{
                align_rot = rotation_align(surface_normal, child_att_orientation);
            }

            align_rot_q = math::from_mat3(align_rot);
            align_rotation = btQuaternion(align_rot_q.q[0], align_rot_q.q[1], align_rot_q.q[2], align_rot_q.q[3]);
            align_rotation = p_transform.getRotation() * align_rotation; // reverse previous inverse rotation
            btv3_child_att = btVector3(child_att.v[0], child_att.v[1], child_att.v[2]);

            btTransform transform_final = btTransform(btQuaternion::getIdentity(), -btv3_child_att);
            btTransform object_T = btTransform(btQuaternion::getIdentity(), hit_point_world);

            transform_final = object_T * btTransform(align_rotation * part->m_user_rotation) * transform_final;

            btVector3 disp = transform_final.getOrigin() - transform_original.getOrigin();
            part->updateSubTreeMotionState(disp, transform_original.getOrigin(),
                                           align_rotation * part->m_user_rotation * rotation.inverse());

            obj->m_body->getMotionState()->getWorldTransform(p_transform);

            BasePart* parent = static_cast<BasePart*>(obj);

            if(lmb_focused_press){
                createConstraint(part, parent, transform_final.inverse() * p_transform);

                m_asset_manager->m_editor_vessel->addChildById(std::dynamic_pointer_cast<BasePart>(part->getSharedPtr()),
                                                               parent->getUniqueId());
                m_asset_manager->m_editor_subtrees.erase(part->getUniqueId());
            }

            if((parent->getClonedFrom() || parent->getClones().size()) && m_symmetry_sides > 1){
                BasePart* cloned_from;  // part that "parent" was cloned from
                std::vector<BasePart*> clone_to;
                if(parent->getClonedFrom()){
                    cloned_from = parent->getClonedFrom();

                    for(uint i=0; i < cloned_from->getClones().size(); i++){
                        if(cloned_from->getClones().at(i) != parent){
                            clone_to.emplace_back(cloned_from->getClones().at(i));
                        }
                    }
                    clone_to.emplace_back(cloned_from);
                }
                else{
                    cloned_from = parent;
                    clone_to = parent->getClones();
                }

                if(m_symmetry_sides != cloned_from->getClones().size() + 1){
                    m_symmetry_sides = cloned_from->getClones().size() + 1;
                    m_editor_gui->setSymmetrySides(m_symmetry_sides);
                }
                createSymmetrySubtrees();
                placeClonedSubtreesOnClones(parent, transform_final, clone_to);
            }
            else{
                createSymmetrySubtrees();

                for(uint i=0; i < m_asset_manager->m_symmetry_subtrees.size(); i++){
                    BasePart* current = m_asset_manager->m_symmetry_subtrees.at(i).get();
                    if(current->m_body.get()){ // check if m_body is initialized, it is not during the first tick
                        btQuaternion symmetric_rotation, rotation_current;
                        btTransform symmetric_rotation_transform;

                        current->m_body->getMotionState()->getWorldTransform(transform_original);
                        rotation_current = transform_original.getRotation();
                        getUserRotation(user_rotation, rotation_current);

                        symmetric_rotation.setEulerZYX(0.0, (2 * M_PI / m_symmetry_sides) * (i + 1), 0.0);
                        symmetric_rotation_transform = btTransform(symmetric_rotation);

                        transform_final = btTransform(btQuaternion::getIdentity(), -btv3_child_att);
                        btVector3 hit_point_world_rotated = object_iR * hit_point_world;
                        hit_point_world_rotated -= object_iR * p_transform.getOrigin();
                        hit_point_world_rotated = symmetric_rotation_transform * hit_point_world_rotated;
                        hit_point_world_rotated = btTransform(p_transform.getRotation()) * (hit_point_world_rotated + (object_iR * p_transform.getOrigin()));

                        object_T = btTransform(btQuaternion::getIdentity(), hit_point_world_rotated);
                        btTransform object_R = btTransform(rotation_current);

                        transform_final = object_T * object_R * transform_final;
                        disp = transform_final.getOrigin() - transform_original.getOrigin();

                        current->updateSubTreeMotionState(disp, transform_original.getOrigin(),
                                                          align_rotation * symmetric_rotation * 
                                                          part->m_user_rotation * rotation_current.inverse());

                        if(lmb_focused_press){
                            createConstraint(current, parent, transform_final.inverse() * p_transform);

                            m_asset_manager->m_editor_vessel->addChildById(std::dynamic_pointer_cast<BasePart>(current->getSharedPtr()),
                                                                           parent->getUniqueId());
                        }
                    }
                }
            }

            if(lmb_focused_press){
                m_asset_manager->m_symmetry_subtrees.clear();
            }
        }
        else{
            clearSymmetrySubtrees();

            m_camera->castRayMousePos(10.f, ray_start_world, ray_end_world);
            btVector3 origin(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]);
            btVector3 disp = origin - transform_original.getOrigin();
            part->updateSubTreeMotionState(disp, transform_original.getOrigin(),
                                           part->m_user_rotation * rotation.inverse());
        }
    }
}


void GameEditor::pickAttachedObject(BasePart* part){
    if(part->getClonedFrom()){
        part = part->getClonedFrom();
    }

    if(part->getClones().size()){
        // temp vector copy because clearSubTreeCloneData will delete the pointers from the part's clone vector while we iterate over it
        std::vector<BasePart*> temp = part->getClones();

        for(uint i=0; i < temp.size(); i++){
            BasePart* clone = temp.at(i);

            clone->clearSubTreeCloneData();
            m_asset_manager->removePartConstraint(clone);
            m_asset_manager->deleteSubtree(clone->getVessel()->removeChild(clone));
        }
    }

    m_picked_obj = part;

    if(!part->isRoot()){
        m_asset_manager->removePartConstraint(part);
        m_asset_manager->m_editor_subtrees.insert({part->getUniqueId(), part->getVessel()->removeChild(part)});
    }
}


void GameEditor::pickObject(){
    double mousey, mousex;
    dmath::vec3 ray_start_world, ray_end_world;
    Object* obj;

    m_input->getMousePos(mousex, mousey);
    m_camera->castRayMousePos(1000.f, ray_start_world, ray_end_world);

    btCollisionWorld::ClosestRayResultCallback ray_callback(btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                                            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
    ray_callback.m_collisionFilterGroup = CG_RAY_EDITOR_SELECT;

    obj = m_physics->testRay(ray_callback, 
                                btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
    if(obj){
        BasePart* part = static_cast<BasePart*>(obj);

        if(part->getVessel() == nullptr){ // not a vessel
            while(part->getParent() != nullptr){
               part = part->getParent();
            }
            m_picked_obj = part;
        }
        else{
            if(m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT)){ // clone
                if(!part->isRoot()){
                    std::shared_ptr<BasePart> clone;
                    part->cloneSubTree(clone, true, false);
                    m_asset_manager->m_editor_subtrees.insert({clone->getUniqueId(), clone});
                    m_picked_obj = clone.get();
                }
            }
            else{
                pickAttachedObject(part);
            }
        }
    }
}


void GameEditor::processGUI(){
    // GUI processing
    if(m_gui_action == EDITOR_ACTION_OBJECT_PICK && m_physics_pause){
        const std::unique_ptr<BasePart>* editor_picked_object = m_editor_gui->getPickedObject();
        std::shared_ptr<BasePart> part(editor_picked_object->get()->clone());

        if(m_picked_obj){ // if the user has an scene object picked just leave it "there"
            m_picked_obj = nullptr;
        }

        if(!m_vessel_id){ // set the vessel root
            m_asset_manager->addBody(part.get(), btVector3(0.0, 30.0, 0.0),
                                     btVector3(0.0, 0.0, 0.0), btQuaternion::getIdentity());

            std::shared_ptr<Vessel> vessel = std::make_shared<Vessel>(std::move(part), m_input);
            m_vessel_id = vessel->getId();
            m_asset_manager->m_editor_vessel = std::move(vessel);
            m_asset_manager->m_editor_vessel->getRoot()->setCollisionFilters(
                m_asset_manager->m_editor_vessel->getRoot()->getCollisionFilters() | CG_RAY_EDITOR_RADIAL);
        }
        else{
            dmath::vec3 ray_start_world, ray_end_world;
            m_camera->castRayMousePos(10.f, ray_start_world, ray_end_world);
            m_asset_manager->addBody(part.get(), btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]),
                                     btVector3(0.0, 0.0, 0.0), btQuaternion::getIdentity());

            m_asset_manager->m_editor_subtrees.insert({part->getUniqueId(), part});
            m_picked_obj = part.get();
        }
    }

    if(m_gui_action == EDITOR_ACTION_DELETE && m_physics_pause){
        m_delete_current = true;
    }

    if(m_gui_action == EDITOR_ACTION_SYMMETRY_SIDES){
        m_symmetry_sides = m_editor_gui->getSymmetrySides();
    }

    if(m_gui_action == EDITOR_ACTION_TOGGLE_ALIGN){
        m_radial_align = m_editor_gui->getRadialAlign();
    }

    if(m_gui_action == EDITOR_ACTION_CLEAR_SCENE){
        m_clear_scene = true;
    }

    if(m_gui_action == EDITOR_ACTION_ADD_STAGE){
        uint pos = m_editor_gui->getNewStagePos();
        if(m_asset_manager->m_editor_vessel)
            m_asset_manager->m_editor_vessel->addEmptyStage(pos);
    }

    if(m_gui_action == EDITOR_ACTION_REMOVE_STAGE){
        uint pos = m_editor_gui->getNewStagePos();
        if(m_asset_manager->m_editor_vessel)
            m_asset_manager->m_editor_vessel->removeEmptyStage(pos);
    }

    if(m_gui_action == EDITOR_ACTION_LAUNCH){
        m_exit_editor = true;
        m_exit_code = EXIT_LAUNCH;
    }

    if(m_gui_action == EDITOR_ACTION_EXIT){
        m_exit_editor = true;
        m_exit_code = EXIT_MAIN_MENU;
    }
}


void GameEditor::processInput(){
    double scx, scy;
    m_input->getScroll(scx, scy);

    if(!m_render_context->imGuiWantCaptureMouse()){
        if(scy && !m_gui_action){
            m_camera->incrementOrbitalCamDistance(-scy * 5.0); // we should check the camera mode
        }

        if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_2] & INPUT_MBUTTON_RELEASE &&
            !m_gui_action && m_camera->getPrevInputMode() != GLFW_CURSOR_DISABLED){
            onRightMouseButton();
        }
    }

    // keyboard input
    if(!m_render_context->imGuiWantCaptureKeyboard()){
        if((m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] & (INPUT_KEY_DOWN | INPUT_KEY_REPEAT))
            && (m_input->pressed_keys[GLFW_KEY_C] & INPUT_KEY_DOWN)){

            Vessel* selected_vessel = m_player->getVessel();
            if(selected_vessel)
                m_player->unsetVessel();
            else if(m_asset_manager->m_editor_vessel.get()){
                selected_vessel = m_asset_manager->m_editor_vessel.get();
                m_player->setVessel(selected_vessel);

            }
       }

        if(m_input->pressed_keys[GLFW_KEY_F12] == INPUT_KEY_DOWN){
            m_render_context->toggleDebugOverlay();
        }

        if(m_input->pressed_keys[GLFW_KEY_F11] == INPUT_KEY_DOWN){
            m_render_context->toggleDebugDraw();
        }

        if(m_input->pressed_keys[GLFW_KEY_F] == INPUT_KEY_DOWN){
            m_clear_scene = true;
        }

        if(m_input->pressed_keys[GLFW_KEY_F10] == INPUT_KEY_DOWN){
            m_render_context->reloadShaders();
            m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));
        }

        if(m_input->pressed_keys[GLFW_KEY_X] == INPUT_KEY_DOWN){
            if(m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] != INPUT_KEY_UP && m_symmetry_sides > 1){
                m_symmetry_sides--;
            }
            if(m_input->pressed_keys[GLFW_KEY_LEFT_SHIFT] == INPUT_KEY_UP && m_symmetry_sides < MAX_SYMMETRY_SIDES){
                m_symmetry_sides++;
            }
            m_editor_gui->setSymmetrySides(m_symmetry_sides);
        }

        if(m_input->pressed_keys[GLFW_KEY_V] == INPUT_KEY_DOWN){
            m_radial_align = !m_radial_align;
            m_editor_gui->setRadialAlign(m_radial_align);
        }

        if(m_input->pressed_keys[GLFW_KEY_F1] == INPUT_KEY_DOWN){
            if(m_asset_manager->m_editor_vessel.get()){
                m_asset_manager->m_editor_vessel->printVessel();
            }
        }

        if(m_input->pressed_keys[GLFW_KEY_F2] == INPUT_KEY_DOWN){
            if(m_asset_manager->m_editor_vessel.get()){
                m_asset_manager->m_editor_vessel->printStaging();
            }
        }

        if(m_input->pressed_keys[GLFW_KEY_ESCAPE] == INPUT_KEY_DOWN){
            m_exit_editor = true;
            m_exit_code = EXIT_LAUNCH;
        }

        if(m_input->pressed_keys[GLFW_KEY_P] == INPUT_KEY_DOWN){
            m_physics_pause = !m_physics_pause;
            m_physics->pauseSimulation(m_physics_pause);
            if(m_picked_obj){
                m_picked_obj = nullptr;
            }
        }
    }
}


void GameEditor::setPartHighlight(){
    BasePart* new_highlight = m_editor_gui->getHighlightedPart();

    if(m_highlight_part != nullptr)
        m_highlight_part->setAlpha(1.f);

    if(new_highlight != nullptr){
        new_highlight->setAlpha(.5f);
        m_highlight_part = new_highlight;
    }
}


void GameEditor::logic(){
    m_gui_action = m_editor_gui->update();

    if(!m_physics_pause){ /* Update vessels and parts */
        clearSymmetrySubtrees();
        if(m_asset_manager->m_editor_vessel.get()){
            m_asset_manager->m_editor_vessel->update();
        }
    }

    if(m_input->pressed_mbuttons[GLFW_MOUSE_BUTTON_1] & INPUT_MBUTTON_PRESS &&
      !m_render_context->imGuiWantCaptureMouse()){
        lmb_focused_press = true;
    }
    else{
        lmb_focused_press = false;
    }

    if(m_picked_obj && m_gui_action != EDITOR_ACTION_DELETE){
        BasePart* part = static_cast<BasePart*>(m_picked_obj);

        float closest_dist = 99999999999.9;;
        math::vec4 closest_att_point_world;
        BasePart* closest = nullptr;

        if(m_vessel_id != 0){
            getClosestAtt(closest_dist, closest_att_point_world, closest, part);
        }
        placeSubTree(closest_dist, closest_att_point_world, closest, part);

        if(lmb_focused_press){
            m_picked_obj = nullptr;
        }
    }
    else{ // if not picked object
        if(!m_gui_action && m_physics_pause && lmb_focused_press){ // scene has the focus
            pickObject();
        }
    }

    processGUI();
    processInput();
    setPartHighlight();
}


void GameEditor::clearScene(){
    if(m_picked_obj){
        clearSymmetrySubtrees();
    }    
    m_asset_manager->clearSceneEditor();
    m_vessel_id = 0;
    m_clear_scene = false;
    m_picked_obj = nullptr;
}


void GameEditor::onRightMouseButton(){
    dmath::vec3 ray_start_world, ray_end_world;
    Object* obj;
    BasePart* part;

    m_camera->castRayMousePos(1000.f, ray_start_world, ray_end_world);

    btCollisionWorld::ClosestRayResultCallback ray_callback(btVector3(ray_start_world.v[0], ray_start_world.v[1], ray_start_world.v[2]),
                                                            btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]));
    ray_callback.m_collisionFilterGroup = CG_RAY_EDITOR_SELECT;

    obj = m_physics->testRay(ray_callback, 
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


void GameEditor::deleteCurrent(){
    m_delete_current = false;

    clearSymmetrySubtrees();

    if(!m_picked_obj){
        return;
    }

    m_asset_manager->deleteObjectEditor(static_cast<BasePart*>(m_picked_obj), m_vessel_id);
    m_picked_obj = nullptr;
}

