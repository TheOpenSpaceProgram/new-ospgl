#include "Material.h"
#include <assets/Cubemap.h>

void Uniform::set(Shader* sh, const std::string& name, int* gl_tex) const
{
	if (type == FLOAT)
	{
		sh->setFloat(name, value.as_float);
	}
	else if (type == INT)
	{
		sh->setInt(name, value.as_int);
	}
	else if (type == VEC2)
	{
		sh->setVec2(name, value.as_vec2);
	}
	else if (type == VEC3)
	{
		sh->setVec3(name, value.as_vec3);
	}
	else if (type == VEC4)
	{
		sh->setVec4(name, value.as_vec4);
	}
	else if (type == TEX)
	{
		glActiveTexture(GL_TEXTURE0 + *gl_tex);
		glBindTexture(GL_TEXTURE_2D, value.as_tex->get()->id);

		sh->setInt(name, *gl_tex);

		(*gl_tex)++;
	}
	else
	{
		logger->warn("Attempted to set empty uniform");
	}
}

Uniform::Uniform(float v)
{
	type = FLOAT;
	value.as_float = v;
}

Uniform::Uniform(int v)
{
	type = INT;
	value.as_int = v;
}

Uniform::Uniform(glm::vec2 v)
{
	type = VEC2;
	value.as_vec2 = v;
}

Uniform::Uniform(glm::vec3 v)
{
	type = VEC3;
	value.as_vec3 = v;
}

Uniform::Uniform(glm::vec4 v)
{
	type = VEC4;
	value.as_vec4 = v;
}

Uniform::Uniform(std::string pkg, std::string name)
{
	type = TEX;
	value.as_tex = new AssetHandle<Image>(pkg, name, false);
}



Uniform::~Uniform()
{
	if (type == TEX && value.as_tex != nullptr)
	{
		delete value.as_tex;
	}
}

int Material::set(std::vector<ModelTexture>& assimp_textures, const MaterialOverride& over)
{
	int gl_tex = 0;

	std::unordered_map<std::string, const Uniform*> final_uniforms;

	for(auto it = uniforms.begin(); it != uniforms.end(); it++)
	{
		final_uniforms[it->first] = &it->second;
	}

	for(auto it = over.uniforms.begin(); it != over.uniforms.end(); it++)
	{
		auto pos = final_uniforms.find(it->first);
		if(pos != final_uniforms.end())
		{
			pos->second = &it->second;
		}
	}

	for (auto it = final_uniforms.begin(); it != final_uniforms.end(); it++)
	{
		it->second->set(shader, it->first, &gl_tex);
	}

	for (auto it = assimp_textures.begin(); it != assimp_textures.end(); it++)
	{
		ModelTexture::TextureType type = it->first;
		auto translates = model_texture_type_to_uniform.find(type);
		if (translates != model_texture_type_to_uniform.end())
		{
			std::string uniform = translates->second;

			glActiveTexture(GL_TEXTURE0 + gl_tex);
			glBindTexture(GL_TEXTURE_2D, it->get_image()->id);

			shader->setInt(uniform, gl_tex);

			gl_tex++;
		}
	}

	return gl_tex;

}

void Material::set_core(int* gl_tex, const CameraUniforms& cu, glm::dmat4 model, GLint drawable_id)
{

	if (!core_uniforms.mat4_proj.empty())
	{
		shader->setMat4(core_uniforms.mat4_proj, cu.proj);
	}

	if (!core_uniforms.mat4_view.empty())
	{
		shader->setMat4(core_uniforms.mat4_view, cu.view);
	}

	if (!core_uniforms.mat4_camera_model.empty())
	{
		shader->setMat4(core_uniforms.mat4_camera_model, cu.c_model);
	}

	if (!core_uniforms.mat4_proj_view.empty())
	{
		shader->setMat4(core_uniforms.mat4_proj_view, cu.proj_view);
	}

	if (!core_uniforms.mat4_camera_tform.empty())
	{
		shader->setMat4(core_uniforms.mat4_camera_tform, cu.tform);
	}

	if (!core_uniforms.mat4_final_tform.empty())
	{
		glm::dmat4 final_tform = cu.tform * model;
		shader->setMat4(core_uniforms.mat4_final_tform, final_tform);
	}

	if (!core_uniforms.mat4_model.empty())
	{
		shader->setMat4(core_uniforms.mat4_model, model);
	}

	if (!core_uniforms.mat3_normal_model.empty())
	{
		shader->setMat3(core_uniforms.mat3_normal_model, glm::mat3(transpose(inverse(model))));
	}

	if (!core_uniforms.float_far_plane.empty())
	{
		shader->setFloat(core_uniforms.float_far_plane, cu.far_plane);
	}

	if (!core_uniforms.float_f_coef.empty())
	{
		shader->setFloat(core_uniforms.float_f_coef, 2.0f / glm::log2(cu.far_plane + 1.0f));
	}

	if (!core_uniforms.vec3_camera_relative.empty())
	{
		shader->setVec3(core_uniforms.vec3_camera_relative, cu.cam_pos);
	}

	if (!core_uniforms.mat4_deferred_tform.empty())
	{
		// This transform simply brings the vertices to camera coordinates, and also applies the model
		// but not view or projection
		glm::dmat4 final_mat = cu.c_model * model;
		shader->setMat4(core_uniforms.mat4_deferred_tform, (glm::mat4)final_mat);
	}

	if(!core_uniforms.int_drawable_id.empty())
	{
		shader->setInt(core_uniforms.int_drawable_id, drawable_id);
	}

	if(!core_uniforms.int_irradiance.empty())
	{
		glActiveTexture(GL_TEXTURE0 + *gl_tex);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cu.irradiance);
		shader->setInt(core_uniforms.int_irradiance, *gl_tex);
		(*gl_tex)++;
	}

}

Material* load_material(const std::string & path, const std::string & name, const std::string & pkg, const cpptoml::table& cfg)
{
	// Config makes the package magic work
	Config* c = osp->assets->get<Config>(pkg, name);
	
	Material* out = new Material();

	c->read_to(*out, "");

	return out;
}


MeshConfig::MeshConfig()
{
	// Default config for your typical 3d model
	// without vertex colors
	has_pos = true;
	has_nrm = true;
	has_uv0 = true;
	has_uv1 = false;
	has_tgt = true;
	has_cl3 = false;
	has_cl4 = false;
	flip_uv = true;
}

size_t MeshConfig::get_vertex_floats() const
{
	size_t out = 0;

	// We use ternary to avoid weird situations such
	// as has_pos = 3 (it would be true)
	out += has_pos ? 3 : 0;
	out += has_nrm ? 3 : 0;
	out += has_uv0 ? 2 : 0;
	out += has_uv1 ? 2 : 0;
	out += has_tgt ? 6 : 0;
	out += has_cl3 ? 3 : 0;
	out += has_cl4 ? 4 : 0;

	return out;
}

size_t MeshConfig ::get_vertex_size()
{
	return get_vertex_floats() * sizeof(float);
}
