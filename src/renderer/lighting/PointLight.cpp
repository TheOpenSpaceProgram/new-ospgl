#include "PointLight.h"
#include "../../assets/AssetManager.h"
#include <renderer/util/TextureDrawer.h>
#include <renderer/geometry/SphereGeometry.h>

// We use a global object for this
static GLuint vao = 0, vbo = 0, ebo = 0;
static size_t index_count = 0;

PointLight::PointLight()
{
	if (vao == 0)
	{
		SphereGeometry::generate_and_upload(&vao, &vbo, &ebo, &index_count, 8);
	}

	fullscreen = assets->get<Shader>("core", "shaders/light/point_fullscreen.vs");
	sphere = assets->get<Shader>("core", "shaders/light/point_sphere.vs");
	pos = glm::dvec3(0, 0, 0);
	color = glm::vec3(1.0f, 0.9f, 0.9f);
	spec_color = glm::vec3(1.0f, 0.8f, 0.8f);

	// Setup a quick default light
	set_parameters(1.0f, 0.045f, 0.0075f);
}


PointLight::~PointLight()
{
}

float PointLight::calculate_radius(float min_brightness)
{
	// We solve the quadratic equation for radius:
	
	// Bright is the brightest component
	float bright = glm::max(color.x, glm::max(color.y, color.z));

	float A = quadratic;
	float B = linear;
	float C = constant - bright * (1.0f / min_brightness);

	return (-B + sqrt(B *  -4.0f * A * C)) / (2.0f * A);
}

void PointLight::set_parameters(float constant, float linear, float quadratic, float min_brightness)
{
	this->constant = constant;
	this->linear = linear;
	this->quadratic = quadratic;
	this->radius = calculate_radius(min_brightness);
}

float PointLight::get_constant()
{
	return constant;
}

float PointLight::get_linear()
{
	return linear;
}

float PointLight::get_quadratic()
{
	return quadratic;
}

float PointLight::get_radius()
{
	return radius;
}

void PointLight::do_pass(CameraUniforms& cu, GBuffer* gbuf)
{

	// Sun poo relative to the camera
	glm::dvec3 rel_pos = pos - cu.cam_pos;


	float dist = (float)glm::length(rel_pos);

	// We have a small margin
	if (dist <= radius + 0.001f)
	{
		set_shader_uniforms(fullscreen, gbuf, rel_pos);
		texture_drawer->issue_fullscreen_rectangle();
	}
	else
	{
		set_shader_uniforms(sphere, gbuf, rel_pos);

		// Set projection stuff
		glm::dmat4 model = glm::dmat4(1.0f);
		model = glm::translate(model, rel_pos);
		model = glm::scale(model, glm::dvec3(radius));

		sphere->setMat4("tform", (glm::mat4)(cu.proj_view * model));
		sphere->setVec2("screen_size", cu.screen_size);

		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

}

void PointLight::set_shader_uniforms(Shader* sh, GBuffer* gbuffer, glm::dvec3 rel_pos)
{
	// We don't do irradiance here
	prepare_shader(sh, gbuffer, 0);
	sh->setVec3("light_pos", rel_pos);
	sh->setVec3("color", color);
	sh->setVec3("spec_color", spec_color);
	sh->setFloat("p_constant", constant);
	sh->setFloat("p_linear", linear);
	sh->setFloat("p_quadratic", quadratic);
}
