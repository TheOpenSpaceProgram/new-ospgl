#include "SunLight.h"
#include "../../assets/AssetManager.h"
#include <renderer/util/TextureDrawer.h>


static void make_shadow_fbuffer(GLuint& fbo, GLuint& tex, int SIZE)
{
	glGenFramebuffers(1, &fbo);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SIZE, SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  	float border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);  
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, tex, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);  

}


SunLight::SunLight(int far_size, int near_size)
{
	near_shadow_size = near_size;
	far_shadow_size = far_size;

	shader = assets->get<Shader>("core", "shaders/light/sunlight.vs");
	color = glm::vec3(1.0f, 0.9f, 0.9f);
	spec_color = glm::vec3(1.0f, 0.8f, 0.8f);
	ambient_color = glm::vec3(0.1f, 0.1f, 0.1f);

	// Create the shadow framebuffers
	make_shadow_fbuffer(far_shadow_fbo, far_shadow_tex, far_size);
	make_shadow_fbuffer(near_shadow_fbo, near_shadow_tex, near_size);

	// Reasonable value 
	near_shadow_span = 50.0;	
	position = glm::dvec3(0, 0, 0);
}


SunLight::~SunLight()
{
}

void SunLight::do_pass(CameraUniforms& cu, GBuffer* gbuf)
{
	prepare_shader(shader, gbuf, cu.irradiance);

	// Sun pso relative to the camera
	glm::dvec3 sun_pos = position - cu.cam_pos;
	shader->setVec3("sun_pos", sun_pos);
	shader->setVec3("color", color);
	shader->setVec3("spec_color", spec_color);
	shader->setVec3("ambient_color", ambient_color);

//	glm::dmat4 pos_mat = glm::translate(glm::dmat4(1.0), -position);
	glm::dmat4 pos_mat = glm::dmat4(1.0);

	glm::dmat4 near_shadow_tform = near_shadow_cam.tform * pos_mat * glm::inverse(cu.c_model);

	shader->setMat4("near_shadow_tform", near_shadow_tform);
	// Keep in mind space used by gbuffer textures
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, near_shadow_tex);
	shader->setInt("near_shadow_map", 5);

	texture_drawer->issue_fullscreen_rectangle();
}


ShadowCamera SunLight::get_shadow_camera(glm::dvec3 camera_pos)
{
	float far_plane = near_shadow_span * 4.0;
	near_shadow_cam.proj = glm::ortho(-near_shadow_span, near_shadow_span, -near_shadow_span, near_shadow_span, 1.0, (double)far_plane);
	// TODO: Maybe a fix for when the camera is on the positive y position? Rare but could happen!
	glm::dvec3 dir = glm::normalize(camera_pos - position);
	glm::dvec3 sun_pos = camera_pos - dir * near_shadow_span * 2.0;
	near_shadow_cam.view = glm::lookAt(glm::dvec3(0, 0, 0), dir, glm::dvec3(0.0, 1.0, 0.0));

	near_shadow_cam.c_camera = glm::translate(glm::dmat4(1.0), -sun_pos);
	near_shadow_cam.fbuffer = near_shadow_fbo;
	near_shadow_cam.size = near_shadow_size;
	near_shadow_cam.far_plane = far_plane;	
	near_shadow_cam.cam_pos = sun_pos;

	near_shadow_cam.tform = near_shadow_cam.proj * near_shadow_cam.view * near_shadow_cam.c_camera;

	return near_shadow_cam;

}
