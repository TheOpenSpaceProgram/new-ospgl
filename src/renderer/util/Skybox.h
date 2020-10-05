#pragma once
#include "../Drawable.h"
#include <assets/AssetManager.h>
#include <assets/Cubemap.h>
#include <assets/Shader.h>

class Skybox : public Drawable
{
private:

	AssetHandle<Shader> shader;
	GLuint vbo{}, vao{};

public:
	AssetHandle<Cubemap> cubemap;

	float intensity;

	explicit Skybox(AssetHandle<Cubemap>&& ncubemap);
	~Skybox();

	void forward_pass(CameraUniforms &cu) override;

	bool needs_forward_pass() override { return true; }

	int get_forward_priority() override { return INT_MAX; }

};

