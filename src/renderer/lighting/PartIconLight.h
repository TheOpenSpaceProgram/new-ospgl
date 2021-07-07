#pragma once
#include "Light.h"


class PartIconLight : public Light 
{

private:

	Shader* shader;

public:

	glm::vec3 color;
	glm::dvec3 sun_dir;

	PartIconLight();

	virtual LightType get_type () override { return PART_ICON; }

	virtual void do_pass(CameraUniforms& cu, GBuffer* gbuf) override;

};


