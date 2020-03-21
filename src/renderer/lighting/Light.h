#pragma once
#include <glm/glm.hpp>
#include "../GBuffer.h"
#include "../../assets/Shader.h"
#include "../camera/CameraUniforms.h"

// The GBuffer is always in camera relative space
// (in order to have high-quality floats near the origin)
// So all lights must keep that in mind
class Light
{
private:

	bool is_added;

public:


	virtual void do_pass(CameraUniforms& cu, GBuffer* gbuf) = 0;

	// Useful function as this code is present on all lights
	void prepare_shader(Shader* shader, GBuffer* gbuffer)
	{
		shader->use();
		// Draw the gbuffer and copy depth 
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gbuffer->g_pos);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gbuffer->g_nrm);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, gbuffer->g_col);

		shader->setInt("gPosition", 0);
		shader->setInt("gNormal", 1);
		shader->setInt("gAlbedoSpec", 2);
	}

	void set_added(bool value)
	{
		this->is_added = value;
	}


	bool is_added_to_renderer()
	{
		return is_added;
	}

	Light()
	{
		is_added = false;
	}
};

