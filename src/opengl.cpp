#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <iostream>
#include <ctime>
#include <string>
#include <cmath>
#include <sstream>
#include <algorithm>

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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


int main(){
	int view_mat_location, proj_mat_location, light_pos_location;
	math::vec3 inital_position = math::vec3(-0.0f, 50.0f, 50.0f);
	GLFWwindow *g_window = nullptr;
	int gl_width = 640, gl_height = 480;

	if(change_cwd_to_selfpath() == EXIT_FAILURE)
		std::cerr << "Could not change the cwd to executable path, proceeding" << std::endl;

	log_start();

	if(init_gl(&g_window, gl_width, gl_height) == EXIT_FAILURE)
		return EXIT_FAILURE;
	log_gl_params();

	Input input;
	Camera camera(&inital_position, 67.0f, (float)gl_width / (float)gl_height , 0.1f, 10000000.0f, &input);
	WindowHandler window_handler(g_window, gl_width, gl_height, &input, &camera);
	Frustum frustum;
	DebugOverlay debug_overlay(&camera, &window_handler);
	camera.setSpeed(10.f);
	camera.setWindow(window_handler.getWindow());

	/////////////////////////////////// shader

	GLuint shader_programme = create_programme_from_files("../shaders/phong_blinn_color_vs.glsl",
														  "../shaders/phong_blinn_color_fs.glsl");
	log_programme_info(shader_programme);
	view_mat_location = glGetUniformLocation(shader_programme, "view");
	proj_mat_location = glGetUniformLocation(shader_programme, "proj");
	light_pos_location = glGetUniformLocation(shader_programme, "light_position_world");

	glUseProgram(shader_programme);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, camera.getViewMatrix().m);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, camera.getProjMatrix().m);
	glUniform3fv(light_pos_location, 1, math::vec3(150.0, 100.0, 0.0).v);

	//////////////////////////////////////

	Model* cube_model = new Model("../data/cube.dae", nullptr, shader_programme, &frustum, math::vec3(0.5, 0.0, 0.5));
	Model* terrain_model = new Model("../data/bigcube.dae", nullptr, shader_programme, &frustum, math::vec3(0.75, 0.75, 0.75));
    Model* sphere_model = new Model("../data/sphere.dae", nullptr, shader_programme, &frustum, math::vec3(0.75, 0.75, 0.75));

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

    ///////////////////////////////////////


	glClearColor(0.2, 0.2, 0.2, 1.0);
    bool physics_pause = true;
	while (!glfwWindowShouldClose(window_handler.getWindow())){
		input.update();
		camera.update();
		window_handler.update();
		frustum.extractPlanes(camera.getViewMatrix(), camera.getProjMatrix(), false);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2, 0.2, 0.2, 1.0);

        /// bullet simulation step
        // this way we tie the simulation update rate to the framerate, should be 60hz if we limit it to 60 fps. We should manage the physics in a different thread and limit it to 60 hz
        // to test this we can unlock the fps and see what happens
        if(input.pressed_keys[GLFW_KEY_P]){
            physics_pause = !physics_pause;
        }
        if(!physics_pause)
            bt_wrapper.stepSimulation(1.f / 60.f, 1);

		// rendering
        glUseProgram(shader_programme);
		if(camera.hasMoved())
			glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, camera.getViewMatrix().m);
		if(camera.projChanged())
			glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, camera.getProjMatrix().m);
		int num_rendered = 0;

        num_rendered += ground->render();
        num_rendered += cube1->render();
        num_rendered += cube2->render();
        num_rendered += sphere1->render();
        num_rendered += sphere2->render();

        /////////////////////////////

		debug_overlay.setRenderedObjects(num_rendered);
		debug_overlay.render();

		glfwSwapBuffers(window_handler.getWindow());
	}

    delete ground;
    delete cube1;
    delete cube2;
    delete sphere1;
    delete sphere2;
    
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

    /////////////

	log("Terminating GLFW and exiting");
	std::cout << "Terminating GLFW and exiting" << std::endl;
	glfwTerminate();

	return EXIT_SUCCESS;
}



