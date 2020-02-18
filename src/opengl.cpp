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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <algorithm>

int main(){
	int model_mat_location, view_mat_location, proj_mat_location, model_mat_location_sphere,
	    view_mat_location_sphere, proj_mat_location_sphere ;
	math::vec3 inital_position = math::vec3(0.0f, 0.0f, 5.0f);
	GLFWwindow *g_window = nullptr;
	int gl_width = 640, gl_height = 480;

	if(change_cwd_to_selfpath() == EXIT_FAILURE)
		std::cerr << "Could not change the cwd to executable path, proceeding" << std::endl;

	log_start();

	if(init_gl(&g_window, gl_width, gl_height) == EXIT_FAILURE)
		return EXIT_FAILURE;
	log_gl_params();

	/////////////////////////////////// model
	int num_vertex;
	GLuint vao_model;
	load_scene(std::string("../data/duck.dae"), vao_model, num_vertex);

	/////////////////////////////////// 1m sphere
	int num_vertex_sphere;
	GLuint vao_sphere;
	load_scene(std::string("../data/sphere.dae"), vao_sphere, num_vertex_sphere);

	/////////////////////////////////// shader
	GLuint shader_programme = create_programme_from_files("../shaders/phong_blinn_vs.glsl",
														  "../shaders/phong_blinn_fs.glsl");
	log_programme_info(shader_programme);
	model_mat_location = glGetUniformLocation(shader_programme, "model");
	view_mat_location = glGetUniformLocation(shader_programme, "view");
	proj_mat_location = glGetUniformLocation(shader_programme, "proj");

	/////////////////////////////////// simple shader
	GLuint shader_programme_simple = create_programme_from_files("../shaders/simple_vs.glsl",
										    					 "../shaders/simple_fs.glsl");
	log_programme_info(shader_programme_simple);
	model_mat_location_sphere = glGetUniformLocation(shader_programme_simple, "model");
	view_mat_location_sphere = glGetUniformLocation(shader_programme_simple, "view");
	proj_mat_location_sphere = glGetUniformLocation(shader_programme_simple, "proj");

	//////////////////////////////////////


	////////////////// text testing /////////////////
	Text2D text(gl_width, gl_height, color{1.0, 1.0, 0.0}, 256, "../data/fonts/Liberastika-Regular.ttf", 15);
	Text2D text2(gl_width, gl_height, color{1.0, 1.0, 0.0}, 256, "../data/fonts/Liberastika-Regular.ttf", 15);
	const GLubyte* vendor = glGetString(GL_VENDOR); // Returns the vendor
	const GLubyte* renderer = glGetString(GL_RENDERER); // Returns a hint to the model
	const GLubyte* gl_version = glGetString(GL_VERSION);
	char modelname[64];
	std::ostringstream oss;
	wchar_t vendor_w[128];
	wchar_t renderer_w[128];
	wchar_t glversion_w[128];
	wchar_t modelname_w[64];
	wchar_t totalmemory_w[32];
	unsigned long long mem_bytes;
	float mem_gb;

	wstrcpy(vendor_w, L"Vendor: ", 128);
	wstrcpy(renderer_w, L"Renderer: ", 128);
	wstrcpy(glversion_w, L"GL version: ", 128);

	ucs2wcs(vendor_w+8, vendor, 128-8);
	ucs2wcs(renderer_w+10, renderer, 128-10);
	ucs2wcs(glversion_w+12, gl_version, 128-12);
	
	text.addString(vendor_w, 15, 25, 1, STRING_DRAW_ABSOLUTE_TL);
	text.addString(renderer_w, 15, 45, 1, STRING_DRAW_ABSOLUTE_TL);
	text.addString(glversion_w, 15, 65, 1, STRING_DRAW_ABSOLUTE_TL);

	get_cpu_model(modelname);
	mbstowcs(modelname_w, modelname, 64);
	text.addString(modelname_w+8, 15, 85, 1, STRING_DRAW_ABSOLUTE_TL);

	mem_bytes = get_sys_memory();
	mem_gb = (float)mem_bytes / 1073741824.;
	oss.precision(3);
	oss << "System memory: " << mem_gb << " GB";
	mbstowcs(totalmemory_w, oss.str().c_str(), 64);
	text.addString(totalmemory_w, 15, 105, 1, STRING_DRAW_ABSOLUTE_TL);

	////////////////// text testing /////////////////

	Input input;
	Camera camera(&inital_position, 67.0f, (float)gl_width / (float)gl_height , 0.1f, 10000000.0f, &input);
	WindowHandler window_handler(g_window, gl_width, gl_height, &input, &camera);
	Frustum frustum;
	camera.setSpeed(10.f);

	glfwSetWindowUserPointer(window_handler.getWindow(), &window_handler);
	glfwSetKeyCallback(window_handler.getWindow(), WindowHandler::glfwKeyCallback);
	glfwSetFramebufferSizeCallback(window_handler.getWindow(), WindowHandler::glfwFramebufferSizeCallback);

	glUseProgram(shader_programme);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, camera.getViewMatrix().m);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, camera.getProjMatrix().m);

	glUseProgram(shader_programme_simple);
	glUniformMatrix4fv(view_mat_location_sphere, 1, GL_FALSE, camera.getViewMatrix().m);
	glUniformMatrix4fv(proj_mat_location_sphere, 1, GL_FALSE, camera.getProjMatrix().m);
	
	glClearColor(0.2, 0.2, 0.2, 1.0);

	float rotation_angle = -90.f;
	mat4 loc[10];
	mat4 rotation = rotate_x_deg(identity_mat4(), rotation_angle);
	for(int i = 0; i < 10; i++){
		loc[i] = translate(identity_mat4(), math::vec3(0., 0., i*std::sin(i*0.75)));
		loc[i] = translate(loc[i], math::vec3(i*std::cos(i*0.75), 0., 0.)) * rotation;
	}
	mat4 scale = identity_mat4();
	scale.m[0] = 0.994406;
	scale.m[5] = 0.994406;
	scale.m[10] = 0.994406;

	while (!glfwWindowShouldClose(window_handler.getWindow())){
		glfwPollEvents();
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
		glBindVertexArray(vao_model);
		int num_rendered = 0;
		for(int i=10; i < 1; i++){
			if(frustum.checkSphere(math::vec3(loc[i].m[12], loc[i].m[13], loc[i].m[14]), 2*0.994406)){
				num_rendered += 1;
    			glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, loc[i].m);
				glDrawElements(GL_TRIANGLES, num_vertex * 3, GL_UNSIGNED_INT, NULL);
			}
		}

		glUseProgram(shader_programme_simple);
		glDisable(GL_CULL_FACE);
		if(camera.hasMoved())
			glUniformMatrix4fv(view_mat_location_sphere, 1, GL_FALSE, camera.getViewMatrix().m);
		if(camera.projChanged())
			glUniformMatrix4fv(proj_mat_location_sphere, 1, GL_FALSE, camera.getProjMatrix().m);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glBindVertexArray(vao_sphere);
		for(int i=0; i < 1; i++){
			glUniformMatrix4fv(model_mat_location_sphere, 1, GL_FALSE, (loc[i] * scale).m);
			glDrawElements(GL_TRIANGLES, num_vertex * 3, GL_UNSIGNED_INT, NULL);
		}
		glDrawElements(GL_TRIANGLES, num_vertex_sphere * 3, GL_UNSIGNED_INT, NULL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_CULL_FACE);

		//text rendering test
		glDisable(GL_DEPTH_TEST);

		wchar_t buffer[64];
		text2.clearStrings();
		std::ostringstream oss2;
		oss2 << "Num rendered ducks: " << num_rendered;
		mbstowcs(buffer, oss2.str().c_str(), 64);
		text2.addString(buffer, 15, 15, 1, STRING_DRAW_ABSOLUTE_BL);

		if(camera.projChanged()){
			int w, h;
			window_handler.getFramebufferSize(w, h);
			text.onFramebufferSizeUpdate(w, h);
			text2.onFramebufferSizeUpdate(w, h);
		}
		text.render();
		text2.render();
		glEnable(GL_DEPTH_TEST);
		
		glfwSwapBuffers(window_handler.getWindow());
		update_fps_counter(window_handler.getWindow());
		
	}

	log("Terminating GLFW and exiting");
	std::cout << "Terminating GLFW and exiting" << std::endl;
	glfwTerminate();

	return EXIT_SUCCESS;
}



