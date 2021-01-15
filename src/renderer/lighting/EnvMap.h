#pragma once
#include "Light.h"

class EnvMap : public Light
{
private:

	Shader* shader;

public:

	LightType get_type () override { return ENV_MAP; }

	void do_pass(CameraUniforms& cu, GBuffer* gbuf) override;

	EnvMap();
};


