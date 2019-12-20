#include "Material.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

void Uniform::set(Shader* sh, const std::string& name, int* gl_tex)
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
	if (type == TEX)
	{
		delete value.as_tex;
	}
}

void Material::set(std::vector<AssimpTexture>* assimp_textures)
{
	std::unordered_map<std::string, AssetHandle<Image>*> replaced_textures;

	if (assimp_textures != nullptr)
	{
		for (AssimpTexture& atex : *assimp_textures)
		{
			auto it = assimp_texture_type_to_uniform.find(atex.first);
			if (it != assimp_texture_type_to_uniform.end())
			{
				replaced_textures[it->second] = &atex.second;
			}
		}
	}

	int gl_tex = 0;

	for (auto it = uniforms.begin(); it != uniforms.end(); it++)
	{
		auto replaced = replaced_textures.find(it->first);

		if (replaced == replaced_textures.end())
		{
			it->second.set(shader, it->first, &gl_tex);
		}
		else
		{
			glActiveTexture(GL_TEXTURE0 + gl_tex);
			glBindTexture(GL_TEXTURE_2D, replaced->second->get()->id);

			shader->setInt(it->first, gl_tex);

			gl_tex++;
		}
	}
}

void Material::set_core(const CameraUniforms& cu, glm::dmat4 model)
{
	if (mat4_proj != "")
	{
		shader->setMat4(mat4_proj, cu.proj);
	}

	if (mat4_view != "")
	{
		shader->setMat4(mat4_view, cu.view);
	}

	if (mat4_camera_model != "")
	{
		shader->setMat4(mat4_camera_model, cu.c_model);
	}

	if (mat4_proj_view != "")
	{
		shader->setMat4(mat4_proj_view, cu.proj_view);
	}

	if (mat4_camera_tform != "")
	{
		shader->setMat4(mat4_camera_tform, cu.tform);
	}

	if (mat4_model != "")
	{
		glm::dvec3 scale;
		glm::dquat rotation;
		glm::dvec3 translation;
		glm::dvec3 skew;
		glm::dvec4 perspective;
		glm::decompose(model, scale, rotation, translation, skew, perspective);

		glm::dmat4 final_matrix = glm::translate(model, -translation + (translation - cu.cam_pos));

		shader->setMat4(mat4_model, final_matrix);
	}

	if (float_far_plane != "")
	{
		shader->setFloat(float_far_plane, cu.far_plane);
	}

	if (vec3_camera_relative != "")
	{
		shader->setVec3(vec3_camera_relative, cu.cam_pos);
	}

}

Material* loadMaterial(const std::string & path, const std::string & name, const std::string & pkg, const cpptoml::table& cfg)
{
	// Config makes the package magic work
	Config* c = assets->get<Config>(pkg, name);
	
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
	has_tex = true;
	has_tgt = true;
	has_cl3 = false;
	has_cl4 = false;
	flip_uv = true;
}

size_t MeshConfig::get_vertex_floats()
{
	size_t out = 0;

	// We use ternary to avoid weird situations such
	// as has_pos = 3 (it would be true)
	out += has_pos ? 3 : 0;
	out += has_nrm ? 3 : 0;
	out += has_tex ? 2 : 0;
	out += has_tgt ? 6 : 0;
	out += has_cl3 ? 3 : 0;
	out += has_cl4 ? 4 : 0;

	return out;
}

size_t MeshConfig ::get_vertex_size()
{
	return get_vertex_floats() * sizeof(float);
}
