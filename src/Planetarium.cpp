#include "Planetarium.hpp"
#include "Model.hpp"
#include "maths_funcs.hpp"


Planetarium::Planetarium() : BaseApp(){
    init();
}


Planetarium::Planetarium(int gl_width, int gl_height) : BaseApp(gl_width, gl_height){
    init();
}


void Planetarium::init(){
    m_render_context->setLightPosition(math::vec3(150.0, 100.0, 0.0));

    m_def_font_atlas.reset(new FontAtlas(256));
    m_def_font_atlas->loadFont("../data/fonts/Liberastika-Regular.ttf", 15);
    m_def_font_atlas->loadCharacterRange(32, 255); // ascii
    m_def_font_atlas->loadCharacterRange(913, 1023); // greek and coptic
    m_def_font_atlas->createAtlas(false);
}


Planetarium::~Planetarium(){
}


void build_surface(struct planet_surface& surface){
	surface.surface_tree[0].patch_translation = dmath::vec3(0.5, 0.0, 0.0);
	surface.surface_tree[0].scale = 1.0;
	surface.surface_tree[0].base_rotation = dmath::quat_from_axis_rad(0.0, 0.0, 0.0, 0.0);
	surface.surface_tree[0].color = math::vec4(1.0, 0.0, 0.0, 1.0);

	surface.surface_tree[1].patch_translation = dmath::vec3(-0.5, 0.0, 0.0);
	surface.surface_tree[1].scale = 1.0;
	surface.surface_tree[1].base_rotation = dmath::quat_from_axis_rad(M_PI, 0.0, 1.0, 0.0);
	surface.surface_tree[1].color = math::vec4(0.0, 1.0, 0.0, 1.0);

	surface.surface_tree[2].patch_translation = dmath::vec3(0.0, 0.5, 0.0);
	surface.surface_tree[2].scale = 1.0;
	surface.surface_tree[2].base_rotation = dmath::quat_from_axis_rad(M_PI/2, 0.0, 0.0, 1.0);
	surface.surface_tree[2].color = math::vec4(0.0, 0.0, 1.0, 1.0);

	surface.surface_tree[3].patch_translation = dmath::vec3(0.0, -0.5, 0.0);
	surface.surface_tree[3].scale = 1.0;
	surface.surface_tree[3].base_rotation = dmath::quat_from_axis_rad(-M_PI/2, 0.0, 0.0, 1.0);
	surface.surface_tree[3].color = math::vec4(1.0, 0.0, 1.0, 1.0);

	surface.surface_tree[4].patch_translation = dmath::vec3(0.0, 0.0, 0.5);
	surface.surface_tree[4].scale = 1.0;
	surface.surface_tree[4].base_rotation = dmath::quat_from_axis_rad(-M_PI/2, 0.0, 1.0, 0.0);
	surface.surface_tree[4].color = math::vec4(1.0, 1.0, 0.0, 1.0);

	surface.surface_tree[5].patch_translation = dmath::vec3(0.0, 0.0, -0.5);
	surface.surface_tree[5].scale = 1.0;
	surface.surface_tree[5].base_rotation = dmath::quat_from_axis_rad(M_PI/2, 0.0, 1.0, 0.0);
	surface.surface_tree[5].color = math::vec4(0.0, 1.0, 1.0, 1.0);
}


void Planetarium::render_side(const struct surface_node& surface_subtree, GLuint shader, GLuint relative_planet_location, Model& model, math::mat4& planet_transform_world){
	dmath::mat4 dtransform_planet_relative = dmath::identity_mat4();
	math::mat4 transform_planet_relative;
	dtransform_planet_relative = dmath::quat_to_mat4(surface_subtree.base_rotation);
	dtransform_planet_relative = dmath::translate(dtransform_planet_relative, surface_subtree.patch_translation);
	std::copy(dtransform_planet_relative.m, dtransform_planet_relative.m + 16, transform_planet_relative.m);        	

	m_render_context->useProgram(shader);
    glUniformMatrix4fv(relative_planet_location, 1, GL_FALSE, transform_planet_relative.m);

    model.setMeshColor(surface_subtree.color);
	model.render(planet_transform_world);
}


void Planetarium::run(){
	Model base("../data/base.dae", nullptr, m_render_context->getShader(SHADER_PLANET), m_frustum.get(), m_render_context.get(), math::vec3(1.0, 1.0, 1.0));
    dmath::mat4 planet_transform = dmath::identity_mat4();

    struct planet_surface surface;
    build_surface(surface);

	m_camera->setCameraPosition(dmath::vec3(2.0, 0.0, 0.0));

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    GLuint shader = m_render_context->getShader(SHADER_PLANET);
    GLuint relative_planet_location = glGetUniformLocation(shader, "relative_planet");

    while (!glfwWindowShouldClose(m_window_handler->getWindow())){
        m_input->update();
        m_window_handler->update();
        m_frustum->extractPlanes(m_camera->getCenteredViewMatrix(), m_camera->getProjMatrix(), false);
        m_player->update();

        m_render_context->contextUpdatePlanetarium();

        dmath::vec3 cam_translation = m_camera->getCamPosition();


        ////////////////////

        math::mat4 planet_transform_world;
        dmath::mat4 dplanet_transform_world = planet_transform;
        dplanet_transform_world.m[12] -= cam_translation.v[0];
        dplanet_transform_world.m[13] -= cam_translation.v[1];
        dplanet_transform_world.m[14] -= cam_translation.v[2];
    	std::copy(dplanet_transform_world.m, dplanet_transform_world.m + 16, planet_transform_world.m);     

        for(uint i=0; i < 6; i++){
        	render_side(surface.surface_tree[i], shader, relative_planet_location, base, planet_transform_world);
        }

        m_render_context->setLightPosition(math::vec3(cam_translation.v[0], cam_translation.v[1], cam_translation.v[2]));

        glfwSwapBuffers(m_window_handler->getWindow());
    }

    m_window_handler->terminate();
}

