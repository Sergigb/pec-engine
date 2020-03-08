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
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <stb/stb_image.h>

#define BT_USE_DOUBLE_PRECISION
#include <bullet/btBulletDynamicsCommon.h>


int main(){
	int model_mat_location, view_mat_location, proj_mat_location, light_pos_location;
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
	struct bbox aabb_duck;
	unsigned char* data;
	int num_vertex, x, y, n;
	float rad;
	GLuint vao_model, tex_id;
	load_scene(std::string("../data/duck_textured.dae"), vao_model, num_vertex, rad, aabb_duck);
	data = stbi_load("../data/duck_tex.png", &x, &y, &n, 0);
	
    glGenTextures(1, &tex_id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    stbi_image_free(data);

	/////////////////////////////////// shader
	GLuint shader_programme = create_programme_from_files("../shaders/phong_blinn_vs.glsl",
														  "../shaders/phong_blinn_fs.glsl");
	log_programme_info(shader_programme);
	model_mat_location = glGetUniformLocation(shader_programme, "model");
	view_mat_location = glGetUniformLocation(shader_programme, "view");
	proj_mat_location = glGetUniformLocation(shader_programme, "proj");
	light_pos_location = glGetUniformLocation(shader_programme, "light_position_world");

	//////////////////////////////////////


	////////////////// text testing /////////////////
	Text2D text2(gl_width, gl_height, color{1.0, 1.0, 0.0}, 256, "../data/fonts/Liberastika-Regular.ttf", 15);
	Text2D text_fps(gl_width, gl_height, color{1.0, 1.0, 0.0}, 256, "../data/fonts/Liberastika-Regular.ttf", 15);
	Text2D* text;
	debug_info_box(&text, gl_width, gl_height);

	////////////////// text testing /////////////////

	Input input;
	Camera camera(&inital_position, 67.0f, (float)gl_width / (float)gl_height , 0.1f, 10000000.0f, &input);
	WindowHandler window_handler(g_window, gl_width, gl_height, &input, &camera);
	Frustum frustum;
	camera.setSpeed(10.f);
	camera.setWindow(window_handler.getWindow());

	glUseProgram(shader_programme);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, camera.getViewMatrix().m);
	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, camera.getProjMatrix().m);
	glUniform3fv(light_pos_location, 1, camera.getCamPosition().v);
	
	glClearColor(0.2, 0.2, 0.2, 1.0);

	float rotation_angle = -90.f;
	mat4 loc[100];
	mat4 rotation = rotate_x_deg(identity_mat4(), rotation_angle);
	for(int i = 0; i < 100; i++){
		loc[i] = translate(identity_mat4(), math::vec3(0., 0., i*std::sin(i*0.75)));
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
		
		/*if(camera.hasMoved())
			glUniform3fv(light_pos_location, 1, camera.getCamPosition().v); // as a test, the light is on the camera*/

		if(camera.hasMoved())
			glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, camera.getViewMatrix().m);
		if(camera.projChanged())
			glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, camera.getProjMatrix().m);
		glBindVertexArray(vao_model);
		glActiveTexture(GL_TEXTURE0);
    	glBindTexture(GL_TEXTURE_2D, tex_id);
		int num_rendered = 0;
		for(int i=0; i < 100; i++){
			//if(frustum.checkSphere(math::vec3(loc[i].m[12], loc[i].m[13], loc[i].m[14]), 2*0.994406)){
			if(frustum.checkBox(aabb_duck.vert, loc[i])){
				num_rendered += 1;
    			glUniformMatrix4fv(model_mat_location, 1, GL_FALSE, loc[i].m);
				glDrawElements(GL_TRIANGLES, num_vertex * 3, GL_UNSIGNED_INT, NULL);
			}
		}

		//text rendering test
		glDisable(GL_DEPTH_TEST);

		wchar_t buffer[64];
		text2.clearStrings();
		std::ostringstream oss2;
		oss2 << "Num rendered ducks: " << num_rendered;
		mbstowcs(buffer, oss2.str().c_str(), 64);
		text2.addString(buffer, 15, 15, 1, STRING_DRAW_ABSOLUTE_BL);

		oss2.str("");
		oss2.clear();
		oss2 << (int)get_fps() << " FPS";
		mbstowcs(buffer, oss2.str().c_str(), 64);
		text_fps.clearStrings();
		text_fps.addString(buffer, 75, 15, 1, STRING_DRAW_ABSOLUTE_TR);

		if(camera.projChanged()){
			int w, h;
			window_handler.getFramebufferSize(w, h);
			text->onFramebufferSizeUpdate(w, h);
			text2.onFramebufferSizeUpdate(w, h);
			text_fps.onFramebufferSizeUpdate(w, h);
		}
		text->render();
		text2.render();
		text_fps.render();
		glEnable(GL_DEPTH_TEST);
		
		glfwSwapBuffers(window_handler.getWindow());
	}

	log("Terminating GLFW and exiting");
	std::cout << "Terminating GLFW and exiting" << std::endl;
	glfwTerminate();

	return EXIT_SUCCESS;
}



