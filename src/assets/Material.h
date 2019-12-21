#pragma once
#include <glad/glad.h>
#include "AssetManager.h"
#include "Shader.h"
#include "Image.h"
#include "Config.h"
#include "../util/SerializeUtil.h"
#include "../renderer/camera/CameraUniforms.h"
#include <glm/gtx/matrix_decompose.hpp>
#include <algorithm>
#include <iostream>

#include <assimp/material.h>

struct MeshConfig
{
	// Shader uniform ID go in this order:
	//
	// pos: vec3	-> Vertex position
	// nrm: vec3	-> Normal
	// tex: vec2	-> Texture coordinate
	// tgt: 2*vec3	-> Tangent and Bitangent (First tangent, then bitangent)
	// cl3: vec3	-> Vertex color (RGB)
	// cl4: vec4	-> Vertex color (RGBA)
	//
	// If, for example, you only have nrm and cl3
	// then the uniforms will be 0 = nrm and 1 = cl3
	bool has_pos;
	bool has_nrm;
	bool has_tex;
	bool has_tgt;
	bool has_cl3;
	bool has_cl4;

	bool flip_uv;

	// NOTE: It does not make much sense to have both cl3 and cl4 activated
	// but it's possible
	MeshConfig();

	size_t get_vertex_floats();

	size_t get_vertex_size();

};



struct Uniform
{
private:

	enum Type
	{
		FLOAT,
		INT,
		VEC2,
		VEC3,
		VEC4,
		TEX,
		EMPTY
	};

	union Value
	{
		float as_float;
		int as_int;
		glm::vec2 as_vec2;
		glm::vec3 as_vec3;
		glm::vec4 as_vec4;
		AssetHandle<Image>* as_tex;

		Value()
		{
			as_tex = nullptr;
		}
	};

	Type type;
	Value value;

public:

	void set(Shader* sh, const std::string& name, int* gl_tex);

	Uniform(float v);
	Uniform(int v);
	Uniform(glm::vec2 v);
	Uniform(glm::vec3 v);
	Uniform(glm::vec4 v);
	// Texture constructor
	Uniform(std::string pkg, std::string name);

	Uniform()
	{
		type = EMPTY;
	}

	Uniform(Uniform&& b)
	{
		value = b.value;
		type = b.type;

		if (type == TEX)
		{
			b.value.as_tex = nullptr;
		}
	}
	
	Uniform& operator=(Uniform&& other)
	{
		if (this != &other)
		{
			value = other.value;
			type = other.type;

			if (other.type == TEX && other.value.as_tex != nullptr)
			{	
				other.value.as_tex = nullptr;
			}
		}

		return *this;
	}


	~Uniform();
};



using AssimpTexture = std::pair <aiTextureType, AssetHandle<Image>>;

struct CoreUniforms
{
	// Name of the different core uniforms
// By default materials bind
// - model = "model"
// - camera_tform = "camera_tform"
// - proj_view = "proj_view"
// - f_coef = "f_coef"
// - camera_relative = "camera_relative"
// If any equals "", it's not binded
	std::string mat4_proj, mat4_view, mat4_camera_model, mat4_proj_view, mat4_camera_tform, mat4_model,
		mat4_final_tform, float_far_plane, float_f_coef, vec3_camera_relative;

	CoreUniforms()
	{
		// Defaults
		mat4_proj = "";
		mat4_view = "";
		mat4_camera_model = "";
		mat4_proj_view = "proj_view";
		mat4_camera_tform = "camera_tform";
		mat4_model = "model";
		float_far_plane = "";
		float_f_coef = "f_coef";
		mat4_final_tform = "";
		vec3_camera_relative = "camera_relative";
	}
};

struct Material
{

	MeshConfig cfg;

	Shader* shader;

	std::unordered_map<std::string, Uniform> uniforms;

	// Allows you to map textures from assimp to a uniform
	// Make sure you have a default defined in uniforms in case the model
	// doesn't have a material with said texture!
	// Keep in mind that this limits textures to only one per type, but
	// realistically there will be no need for more than that.
	std::unordered_map<aiTextureType, std::string> assimp_texture_type_to_uniform;



	// Name of the different other core uniforms
	CoreUniforms core_uniforms;


	void set();
	void set_core(const CameraUniforms& cu, glm::dmat4 model);

};

Material* loadMaterial(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);

template<>
class GenericSerializer<Material>
{
public:

	static void serialize(const Material& what, cpptoml::table& target)
	{
		logger->check(false, "Not implemented");
	}

	static void obtain_uniforms(Material& to, const cpptoml::table& uniforms)
	{
		for (auto sub : uniforms)
		{
			std::string name = sub.first;
			auto contents = sub.second;

			if (contents->is_table())
			{
				// Can be vec2, vec3 or vec4
				auto table = contents->as_table();

				auto x_opt = table->get_qualified_as<double>("x");
				auto y_opt = table->get_qualified_as<double>("y");
				auto z_opt = table->get_qualified_as<double>("z");
				auto w_opt = table->get_qualified_as<double>("w");

				if (w_opt)
				{
					to.uniforms[name] = Uniform(glm::vec4(*x_opt, *y_opt, *z_opt, *w_opt));
				}
				else if (z_opt)
				{
					to.uniforms[name] = Uniform(glm::vec3(*x_opt, *y_opt, *z_opt));
				}
				else if (y_opt)
				{
					to.uniforms[name] = Uniform(glm::vec2(*x_opt, *y_opt));
				}
				else
				{
					logger->warn("Invalid vector uniform size (name = {})", name);
				}
			}
			else if (contents->is_value())
			{
				// int, float or tex
				auto as_int = contents->as<int64_t>();
				auto as_decimal = contents->as<double>();
				auto as_tex = contents->as<std::string>();

				if (as_int)
				{
					to.uniforms[name] = Uniform((int)as_int->get());
				}
				else if (as_decimal)
				{
					to.uniforms[name] = Uniform((float)as_decimal->get());
				}
				else if (as_tex)
				{
					auto[pkg, name] = assets->get_package_and_name(as_tex->get(), assets->get_current_package());
					to.uniforms[name] = Uniform(pkg, name);
				}
				else
				{
					logger->warn("Unknown uniform type (name = {})", name);
				}
			}
			else
			{
				logger->warn("Unknown uniform type (name = {})", name);
			}
		}
	}

	// Deserialize is only called for bodies and barycenters
	// Star is special
	static void deserialize(Material& to, const cpptoml::table& from)
	{
		std::string str;

		SAFE_TOML_GET(str, "shader", std::string);

		to.shader = assets->get_from_path<Shader>(str);

		auto cfg_toml = from.get_table_qualified("config");
		if (cfg_toml)
		{
			SerializeUtil::read_to(*cfg_toml, to.cfg);
		}
		else
		{
			to.cfg = MeshConfig();
		}

		auto core_toml = from.get_table_qualified("core_uniforms");
		if (core_toml)
		{
			SerializeUtil::read_to(*core_toml, to.core_uniforms);
		}
		else
		{
			to.core_uniforms = CoreUniforms();
		}

		auto uniforms_toml = from.get_table_qualified("uniforms");
		if (uniforms_toml)
		{
			obtain_uniforms(to, *uniforms_toml);
		}

	}
};

template<>
class GenericSerializer<CoreUniforms>
{
public:

	static void serialize(const CoreUniforms& what, cpptoml::table& target)
	{
		logger->check(false, "Not implemented");
	}

	static void deserialize(CoreUniforms& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET_OR(to.mat4_proj, "proj", std::string, "");
		SAFE_TOML_GET_OR(to.mat4_final_tform, "final_tform", std::string, "");
		SAFE_TOML_GET_OR(to.mat4_view, "view", std::string, "");
		SAFE_TOML_GET_OR(to.mat4_camera_model, "camera_model", std::string, "");
		SAFE_TOML_GET_OR(to.mat4_proj_view, "proj_view", std::string, "");
		SAFE_TOML_GET_OR(to.mat4_camera_tform, "camera_tform", std::string, "");
		SAFE_TOML_GET_OR(to.mat4_model, "model", std::string, "");
		SAFE_TOML_GET_OR(to.float_far_plane, "far_plane", std::string, "");
		SAFE_TOML_GET_OR(to.float_f_coef, "f_coef", std::string, "");
		SAFE_TOML_GET_OR(to.vec3_camera_relative, "camera_relative", std::string, "");
	}
};



template<>
class GenericSerializer<MeshConfig>
{
public:

	static void serialize(const MeshConfig& what, cpptoml::table& target)
	{
		logger->check(false, "Not implemented");
	}

	static void deserialize(MeshConfig& to, const cpptoml::table& from)
	{
		SAFE_TOML_GET_OR(to.has_pos, "has_pos", bool, false);
		SAFE_TOML_GET_OR(to.has_nrm, "has_nrm", bool, false);
		SAFE_TOML_GET_OR(to.has_tex, "has_tex", bool, false);
		SAFE_TOML_GET_OR(to.has_tgt, "has_tgt", bool, false);
		SAFE_TOML_GET_OR(to.has_cl3, "has_cl3", bool, false);
		SAFE_TOML_GET_OR(to.has_cl4, "has_cl4", bool, false);
		SAFE_TOML_GET_OR(to.flip_uv, "flip_uv", bool, false);
	}
};
