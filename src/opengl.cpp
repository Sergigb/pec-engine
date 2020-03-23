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

    quat.setEuler(45, 25, 0);
    Object* cube2 = new Object(cube_model, &bt_wrapper, cube_shape, btVector3(0.0, 45.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(1.0));
    cube2->setColor(math::vec3(0.0, 1.0, 0.0));

    quat.setEuler(0, 0, 0);
    Object* sphere1 = new Object(sphere_model, &bt_wrapper, sphere_shape, btVector3(0.0, 50.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(1.0));
    sphere1->setColor(math::vec3(0.0, 0.0, 1.0));

    Object* sphere2 = new Object(sphere_model, &bt_wrapper, sphere_shape, btVector3(0.0, 55.0, 0.0), btVector3(0.0, 0.0, 0.0), quat, btScalar(10.0));
    sphere2->setColor(math::vec3(1.0, 0.0, 1.0));

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
    render_context.m_objects.push_back(sphere1);
    render_context.m_objects.push_back(sphere2);

    ///////////////////////////////////////

    render_context.setLightPosition(math::vec3(150.0, 100.0, 0.0));
    bool physics_pause = true;
	while (!glfwWindowShouldClose(window_handler.getWindow())){
		input.update();
		camera.update();
		window_handler.update();
		frustum.extractPlanes(camera.getViewMatrix(), camera.getProjMatrix(), false);

        // mouse pick test
        if(input.mButtonPressed() && input.pressed_mbuttons[GLFW_MOUSE_BUTTON_1]){
            int w, h;
            double mousey, mousex;
            input.getMousePos(mousex, mousey);
            window_handler.getFramebufferSize(w, h);
            Object* obj;
            obj = bt_wrapper.testMousePick((float)w, (float)h, (float)mousex, (float)h - (float)mousey, camera.getProjMatrix(), camera.getViewMatrix(), 1000.0f);
            if(obj){
                //obj->setColor(math::vec3(1.0, 0.0, 1.0));
                obj->applyTorque(btVector3(500.0, 0.0, 0.0));
                obj->applyCentralForce(btVector3(0.0, 10000.0, 0.0));
            }
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



