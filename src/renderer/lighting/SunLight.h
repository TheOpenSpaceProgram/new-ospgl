#pragma once
#include "Light.h"
// It's actually a point-light but renders a fullscreen quad. It also allows setting ambient light
//
// Shadows:
// 	We need to render two shadow maps.
// 	- FAR: Renders just the closest planetary surface to the camera,
// 		it casts landscape shadows. Projection depends on distance to surface
//	- NEAR: Renders the closest planetary surface and vehicles,
//		uses a way smaller frustrum
class SunLight : public Light
{
private:

	int near_shadow_size;
	int far_shadow_size;

	Shader* shader;

public:

	glm::dvec3 position;

	glm::vec3 color;
	glm::vec3 spec_color;
	glm::vec3 ambient_color;


	GLuint far_shadow_fbo;
	GLuint near_shadow_fbo;
	GLuint far_shadow_tex;
	GLuint near_shadow_tex;

	// User set
	double near_shadow_span;
	// Set by the renderer
	double far_shadow_span;

	ShadowCamera near_shadow_cam;
	ShadowCamera far_shadow_cam;

	SunLight(int far_shadow_size = 512, int near_shadow_size = 512);
	~SunLight();

	virtual LightType get_type () override { return SUN; }

	virtual void do_pass(CameraUniforms& cu, GBuffer * gbuf) override;
	virtual ShadowCamera get_shadow_camera(glm::dvec3 camera_pos) override;

	virtual bool casts_shadows () override { return true; }

};

