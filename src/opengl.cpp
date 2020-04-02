#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <iostream>
#include <ctime>
#include <string>
#include <cmath>
#include <sstream>
#include <algorithm>
#include <vector>

#include "log.hpp"
#include "utils.hpp"
#include "gl_utils.hpp"
#include "maths_funcs.hpp"
#include "Camera.hpp"
#include "Input.hpp"
#include "WindowHandler.hpp"
#include "Text2D.hpp"
#include "Frustum.hpp"
#include "DebugOverlay.hpp"
#include "Model.hpp"
#include "BtWrapper.hpp"
#include "Object.hpp"
#include "RenderContext.hpp"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


int main(){
	int gl_width = 640, gl_height = 480;

	if(change_cwd_to_selfpath() == EXIT_FAILURE)
		std::cerr << "Could not change the cwd to executable path, proceeding" << std::endl;

	log_start();

	Input input;
	Camera camera(math::vec3(-0.0f, 50.0f, 50.0f), 67.0f, (float)gl_width / (float)gl_height , 0.1f, 100000.0f, &input);
    camera.setSpeed(10.f);
	WindowHandler window_handler(gl_width, gl_height, &input, &camera);
    camera.setWindow(window_handler.getWindow());
	Frustum frustum;
    RenderContext render_context(&camera, &window_handler);

	//////////////////////////////////////

	Model* cube_model = new Model("../data/cube.dae", nullptr, render_context.getShader(SHADER_PHONG_BLINN_NO_TEXTURE), &frustum, math::vec3(0.5, 0.0, 0.5));
	Model* terrain_model = new Model("../data/bigcube.dae", nullptr, render_context.getShader(SHADER_PHONG_BLINN_NO_TEXTURE), &frustum, math::vec3(0.75, 0.75, 0.75));
    Model* sphere_model = new Model("../data/sphere.dae", nullptr, render_context.getShader(SHADER_PHONG_BLINN_NO_TEXTURE), &frustum, math::vec3(0.75, 0.75, 0.75));

    ////////////// bullet test //////////////

    BtWrapper bt_wrapper(btVector3(0, -9.81, 0));
    btQuaternion quat;

    btCollisionShape* cube_shape_ground = new btBoxShape(btVector3(btScalar(25.), btScalar(25.), btScalar(25.))); // box for now, we need to try a mesh
    btCollisionShape* cube_shape = new btBoxShape(btVector3(1,1,1));
    btCollisionShape* sphere_shape = new btSphereShape(btScalar(1));
    btCollisionShape* cube3m = new btBoxShape(btVector3(3,3,3));

    btAlignedObjectArray<btCollisionShape*> collisionShapes;
    collisionShapes.push_back(cube_shape);
    collisionShapes.push_back(cube_shape_ground);
    collisionShapes.push_back(sphere_shape);
    collisionShapes.push_back(cube3m);

    quat.setEuler(0, 0, 0);
    Object* ground = new Object(terrain_model, &bt_wrapper, cube_shape_ground, btVector3(0.0, 0.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(0.0));

    quat.setEuler(20, 50, 0);
    Object* cube1 = new Object(cube_model, &bt_wrapper, cube3m, btVector3(0.0, 40.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(1.0));
    cube1->setMeshScale(3.0);
    cube1->setColor(math::vec3(1.0, 0.0, 0.0));

    quat.setEuler(0, 0, 0);
    Object* cube2 = new Object(cube_model, &bt_wrapper, cube_shape, btVector3(10.5, 30.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(30.0));
    cube2->setColor(math::vec3(0.5, 0.75, 0.0));

    quat.setEuler(0, 0, 0);
    Object* cube3 = new Object(cube_model, &bt_wrapper, cube_shape, btVector3(12.5, 30.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(30.0));
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

    bt_wrapper.addConstraint(hingeConstraint, true);

    for(int i=0; i<10; i++){
        Object* cube = new Object(cube_model, &bt_wrapper, cube_shape, btVector3(0.0, 55.0 + (i+1)*5, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(10.0));
        cube->setColor(math::vec3(1.0, 0.0, 0.0));
        render_context.m_objects.push_back(cube);
    }

    for(int i=0; i<10; i++){
        Object* sphere = new Object(sphere_model, &bt_wrapper, sphere_shape, btVector3(5.0, 55.0 + (i+1)*5, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(10.0));
        sphere->setColor(math::vec3(0.0, 1.0, 0.0));
        render_context.m_objects.push_back(sphere);
    }

    render_context.m_objects.push_back(ground);
    render_context.m_objects.push_back(cube1);
    render_context.m_objects.push_back(cube2);
    render_context.m_objects.push_back(cube3);

    ///////////////////////////////////////

    render_context.setLightPosition(math::vec3(150.0, 100.0, 0.0));
    bool physics_pause = true, last_rmb = false;
    Object* picked_obj = nullptr;
	while (!glfwWindowShouldClose(window_handler.getWindow())){
		input.update();
		camera.update();
		window_handler.update();
		frustum.extractPlanes(camera.getViewMatrix(), camera.getProjMatrix(), false);

        // mouse pick test
        if(input.mButtonPressed() && input.pressed_mbuttons[GLFW_MOUSE_BUTTON_1]){
            if(!picked_obj && !last_rmb){
                int w, h;
                double mousey, mousex;
                math::vec3 ray_start_world, ray_end_world;
                Object* obj;

                input.getMousePos(mousex, mousey);
                window_handler.getFramebufferSize(w, h);
                camera.castRayMousePos((float)w, (float)h, 1000.f, ray_start_world, ray_end_world);

                obj = bt_wrapper.testRay(ray_start_world, ray_end_world);
                if(obj)
                    picked_obj = obj;
            }
            else if(!last_rmb){
                picked_obj->activate(true);
                picked_obj = nullptr;
            }
            last_rmb = true; // we should we do something to avoid this in the input class
        }
        else
            last_rmb = false;

        if(picked_obj){
            int w, h;
            math::vec3 ray_start_world, ray_end_world;
            btQuaternion rotation;
            btVector3 ray_end_world_btv3;
            
            window_handler.getFramebufferSize(w, h);
            camera.castRayMousePos((float)w, (float)h, 25.f, ray_start_world, ray_end_world);

            ray_end_world_btv3 = btVector3(ray_end_world.v[0], ray_end_world.v[1], ray_end_world.v[2]);
            rotation.setEuler(0, 0, 0);
            picked_obj->setMotionState(ray_end_world_btv3, rotation);
        }

        /// bullet simulation step
        // this way we tie the simulation update rate to the framerate, should be 60hz if we limit it to 60 fps. We should manage the physics in a different thread and limit it to 60 hz
        // to test this we can unlock the fps and see what happens
        if(input.pressed_keys[GLFW_KEY_P]){
            physics_pause = !physics_pause;
        }
        if(!physics_pause)
            bt_wrapper.stepSimulation(1.f / 60.f, 0);

		// rendering
        render_context.render();

		glfwSwapBuffers(window_handler.getWindow());
	}

    delete terrain_model;
    delete cube_model;
    delete sphere_model;

    //delete collision shapes
    for (int j = 0; j < collisionShapes.size(); j++)
    {
        btCollisionShape* shape = collisionShapes[j];
        collisionShapes[j] = 0;
        delete shape;
    }

    for(uint i=0; i<render_context.m_objects.size(); i++){
        delete render_context.m_objects.at(i);
    }
    render_context.m_objects.clear();

    /////////////

    window_handler.terminate();

	return EXIT_SUCCESS;
}



