#pragma once
#include <glad/glad.h>
#include "AssetManager.h"
#include "Shader.h"
#include "Image.h"
#include "Config.h"
#include <util/SerializeUtil.h>
#include <renderer/camera/CameraUniforms.h>

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

	void set(Shader* sh, const std::string& name, int* gl_tex) const;

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


struct AssimpTexture
{
	aiTextureType first;
	AssetHandle<Image> second;

	AssimpTexture()
	{
		this->first = aiTextureType_UNKNOWN;
		this->second = AssetHandle<Image>();
	}

	AssimpTexture(AssimpTexture&& b)
	{
		this->first = b.first;
		this->second = std::move(b.second);
	}

	AssimpTexture(const AssimpTexture& b)
	{
		this->first = b.first;
		this->second = b.second.duplicate();
	}


	AssimpTexture& operator=(AssimpTexture&& b)
	{
		this->first = b.first;
		this->second = std::move(b.second);

		return *this;
	}
};

struct CoreUniforms
{
	// Name of the different core uniforms
	// By default materials bind
	// - final_tform = "final_tform"
	// - proj_view = "proj_view"
	// - f_coef = "f_coef"
	// - camera_relative = "camera_relative"
	// - normal_model = "normal_model"
	// - deferred_tform = "deferred_tform"
	// Using camera_tform and the like is not a good idea as the camera usually is very far from the origin
	// final_tform fixes this as it combines both the model and camera translation, so the end result is
	// near the origin, or if it's far away, it will be actually fara way from the camera
	// If any equals "", it's not bound
	std::string mat4_proj, mat4_view, mat4_camera_model, mat4_proj_view, mat4_camera_tform, mat4_model,
		mat4_deferred_tform,
		mat4_final_tform, mat3_normal_model, float_far_plane, float_f_coef, vec3_camera_relative,
		int_drawable_id;

	// Lightning core uniforms:

	// The direction the sun light comes from. Sun is a point light extremely far away,
	// when we are close to the sun, additional "planetshine" is used
	std::string vec3_sunlight_dir;

	CoreUniforms()
	{
		// Defaults
		mat4_proj = "";
		mat4_view = "";
		mat4_camera_model = "";
		mat4_proj_view = "proj_view";
		mat4_camera_tform = "";
		mat4_model = "";
		mat3_normal_model = "normal_model";
		float_far_plane = "";
		float_f_coef = "f_coef";
		mat4_final_tform = "final_tform";
		vec3_camera_relative = "camera_relative";
		mat4_deferred_tform = "deferred_tform";
		vec3_sunlight_dir = "sunlight_dir";
		int_drawable_id = "";
	}
};

struct MaterialOverride
{
	std::unordered_map<std::string, Uniform> uniforms;
};


struct Material
{

	MeshConfig cfg;

	Shader* shader;
	Shader* shadow_shader;

	std::unordered_map<std::string, Uniform> uniforms;

	// Allows you to map textures from assimp to a uniform
	// Make sure you have a default defined in uniforms in case the model
	// doesn't have a material with said texture!
	// Keep in mind that this limits textures to only one per type, but
	// realistically there will be no need for more than that.
	std::unordered_map<aiTextureType, std::string> assimp_texture_type_to_uniform;



	// Name of the different other core uniforms
	CoreUniforms core_uniforms;


	void set(std::vector<AssimpTexture>& assimp_textures, const MaterialOverride& over);
	void set_core(const CameraUniforms& cu, glm::dmat4 model, GLint drawable_id);

};

Material* load_material(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);


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
		to.shadow_shader = assets->get_from_path<Shader>("core:shaders/shadow.vs"); //< TODO: Maybe allow the user to choose a shader?
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

		auto asssimp_textures_toml = from.get_table_qualified("assimp_textures");
		if (asssimp_textures_toml)
		{
			for (auto entry : *asssimp_textures_toml)
			{
				if (entry.second->is_value() && entry.second->as<std::string>())
				{
					std::string val = entry.second->as<std::string>()->get();

					if (entry.first == "diffuse")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_DIFFUSE] = val;
					}
					else if(entry.first == "specular")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_SPECULAR] = val;
					}
					else if (entry.first == "ambient")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_AMBIENT] = val;
					}
					else if (entry.first == "emissive")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_EMISSIVE] = val;
					}
					else if (entry.first == "height")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_HEIGHT] = val;
					}
					else if (entry.first == "normals")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_NORMALS] = val;
					}
					else if (entry.first == "shininess")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_SHININESS] = val;
					}
					else if (entry.first == "opacity")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_OPACITY] = val;
					}
					else if (entry.first == "displacement")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_DISPLACEMENT] = val;
					}
					else if (entry.first == "lightmap")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_LIGHTMAP] = val;
					}
					else if (entry.first == "reflection")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_REFLECTION] = val;
					}
					// PBR Materials
					else if (entry.first == "base_color")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_BASE_COLOR] = val;
					}
					else if (entry.first == "normal_camera")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_NORMAL_CAMERA] = val;
					}
					else if (entry.first == "emission_color")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_EMISSION_COLOR] = val;
					}
					else if (entry.first == "metalness")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_METALNESS] = val;
					}
					else if (entry.first == "diffuse_roughness")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_DIFFUSE_ROUGHNESS] = val;
					}
					else if (entry.first == "ambient_occlusion")
					{
						to.assimp_texture_type_to_uniform[aiTextureType_AMBIENT_OCCLUSION] = val;
					}
					else
					{
						logger->warn("Unknown assimp texture type '{}'", entry.first);
					}
				}
			}
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
		SAFE_TOML_GET_OR(to.int_drawable_id, "drawable_id", std::string, "");
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
