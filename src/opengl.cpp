#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <iostream>
#include <ctime>
#include <string>
#include <cmath>
#include <sstream>

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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


int main(){
	int view_mat_location, proj_mat_location, light_pos_location;
	math::vec3 inital_position = math::vec3(0.0f, 0.0f, 5.0f);
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

	Model* duck = new Model("../data/duck.dae", nullptr, shader_programme, &frustum);
    duck->setMeshColor(math::vec3(0.8, 0.6, 0.05));
	Model* terrain = new Model("../data/terrain.dae", nullptr, shader_programme, &frustum);
    terrain->setMeshColor(math::vec3(0.75, 0.75, 0.75));

	glClearColor(0.2, 0.2, 0.2, 1.0);

	float rotation_angle = -90.f;
	mat4 loc[10];
	mat4 rotation = rotate_x_deg(identity_mat4(), rotation_angle);
    mat4 terrain_rotation = rotate_x_deg(identity_mat4(), rotation_angle);
	for(int i = 0; i < 10; i++){
		loc[i] = translate(identity_mat4(), math::vec3(0., 5., i*std::sin(i*0.75)));
		loc[i] = translate(loc[i], math::vec3(i*std::cos(i*0.75), 0., 0.)) * rotation;
	}

	while (!glfwWindowShouldClose(window_handler.getWindow())){
		input.update();
		camera.update();
		window_handler.update();
		frustum.extractPlanes(camera.getViewMatrix(), camera.getProjMatrix(), false);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glClearColor(0.2, 0.2, 0.2, 1.0);

		// rendering
        glUseProgram(shader_programme);
		if(camera.hasMoved())
			glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, camera.getViewMatrix().m);
		if(camera.projChanged())
			glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, camera.getProjMatrix().m);
		int num_rendered = 0;
		for(int i=0; i < 10; i++){
			num_rendered += duck->render(loc[i]);
		}

        terrain->render(terrain_rotation);

		debug_overlay.setRenderedObjects(num_rendered);
		debug_overlay.render();

		glfwSwapBuffers(window_handler.getWindow());
	}

    delete terrain;
    delete duck;

	log("Terminating GLFW and exiting");
	std::cout << "Terminating GLFW and exiting" << std::endl;
	glfwTerminate();

	return EXIT_SUCCESS;
}



