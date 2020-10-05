#pragma once
#include <glm/glm.hpp>
#include "../util/GBuffer.h"
#include "../../assets/Shader.h"
#include "../camera/CameraUniforms.h"
#include "ShadowCamera.h"

// The GBuffer is always in camera relative space
// (in order to have high-quality floats near the origin)
// So all lights must keep that in mind
class Light
{
private:

	bool is_added;

public:

	enum LightType
	{
		POINT,
		SUN, 
		PART_ICON
	};


	virtual void do_pass(CameraUniforms& cu, GBuffer* gbuf) = 0;

	// Useful function as this code is present on all lights
	void prepare_shader(Shader* shader, GBuffer* gbuffer, GLuint irradiance)
	{
		shader->use();
		// Draw the gbuffer and copy depth 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gbuffer->g_pos);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gbuffer->g_nrm);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gbuffer->g_col);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, gbuffer->g_pbr);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance);

		shader->setInt("gPosition", 0);
		shader->setInt("gNormal", 1);
		shader->setInt("gAlbedo", 2);
		shader->setInt("gPbr", 3);
		shader->setInt("irradiance_map", 4);
	}

	void set_added(bool value)
	{
		this->is_added = value;
	}


	bool is_added_to_renderer()
	{
		return is_added;
	}

	virtual LightType get_type() = 0;

	virtual ShadowCamera get_shadow_camera(glm::dvec3 camera_pos) { return ShadowCamera(); };
	virtual ShadowCamera get_far_shadow_camera() { return ShadowCamera(); }; //< Only on SunLight
	virtual bool casts_shadows() { return false; }

	virtual bool needs_fullscreen_viewport() { return true; }

	Light()
	{
		is_added = false;
	}
};

