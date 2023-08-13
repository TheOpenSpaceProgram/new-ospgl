#pragma once
#include "Light.h"

class DebugGBuffer : public Light
{
private:

	Shader* shader;

public:

	enum View
	{
		NONE,
		POS,
		EMIT,
		EMIT_ALBEDO,
		NORMAL,
		ALBEDO,
		OCCLUSSION,
		METALLIC,
		ROUGHNESS,
		END_MARKER
	};

	const static char* get_mode_str(DebugGBuffer::View v);
	View mode;

	LightType get_type () override { return DEBUG_GBUFFER; }

	void do_pass(CameraUniforms& cu, GBuffer* gbuf) override;

	DebugGBuffer();
};


