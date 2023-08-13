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
		ENV_MAP,
		SUN, 
		PART_ICON,
		DEBUG_GBUFFER
	};


	virtual void do_pass(CameraUniforms& cu, GBuffer* gbuf) = 0;

	// Useful function as this code is present on all lights
	void prepare_shader(Shader* shader, GBuffer* gbuffer,
					 GLuint irradiance, GLuint specular, GLuint brdf)
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

		if(irradiance > 0)
		{
			glActiveTexture(GL_TEXTURE4);
			glBindTexture(GL_TEXTURE_CUBE_MAP, irradiance);
			glActiveTexture(GL_TEXTURE5);
			glBindTexture(GL_TEXTURE_CUBE_MAP, specular);
			glActiveTexture(GL_TEXTURE6);
			glBindTexture(GL_TEXTURE_2D, brdf);

			shader->setInt("irradiance_map", 4);
			shader->setInt("specular_map", 5);
			shader->setInt("brdf_map", 6);
		}
		shader->setInt("gPosition", 0);
		shader->setInt("gNormal", 1);
		shader->setInt("gAlbedo", 2);
		shader->setInt("gPbr", 3);

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

	// Planetary lights effect env_map
	// TODO: Change terminology?
	virtual bool is_planetary_light() { return false; }

	Light()
	{
		is_added = false;
	}
};

