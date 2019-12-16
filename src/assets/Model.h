#pragma once
#include <glad/glad.h>
#include "AssetManager.h"
#include "Shader.h"

struct aiScene;
struct aiNode;
struct aiMesh;

struct ModelConfig
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
	//
	bool has_pos;
	bool has_nrm;
	bool has_tex;
	bool has_tgt;
	bool has_cl3;
	bool has_cl4;

	bool flip_uv;

	// NOTE: It does not make much sense to have both cl3 and cl4 activated
	// but it's possible
	ModelConfig();

	size_t get_vertex_floats();

	size_t get_vertex_size();


	unsigned int get_assimp_flags();

};

class Mesh 
{
private:

	ModelConfig cfg;

	friend class Model;

	GLuint vbo, vao, ebo;

	size_t data_size;
	size_t index_count;

	float* data;
	uint32_t* indices;

	// It's nullptr if the material didn't specify one
	// in its name
	Shader* shader;

public:

	void upload();
	void unload();

	// Only issues the draw command, does absolutely nothing else
	void draw();

	Mesh(ModelConfig c) : cfg(c)
	{
		vao = 0;
		vbo = 0;
		ebo = 0;
		data = nullptr;
		indices = nullptr;
		shader = nullptr;
	}

};

struct Node
{
	std::vector<Mesh> meshes;
	glm::mat4 sub_transform;

	std::vector<Node*> children;
};

// Models allow loading 3d models using the assimp library
// For now we support (and have tested):
// - .blend (Blender files)
// It supports a full hierarchy of meshes, each with a subtransform.
// An empty mesh is a node, it contains only a transform
class Model
{
private:

	void process_node(aiNode* node, const aiScene* scene, Node* to);
	void process_mesh(aiMesh* mesh, const aiScene* scene, Node* to);

	friend class Mesh;

	ModelConfig config;

	int gpu_users;

	bool uploaded;

	Node* root;

	void upload();
	void unload();

public:

	void get_gpu();
	void free_gpu();

	Model(ModelConfig config, const aiScene* scene);
	~Model();
};

Model* loadModel(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);

// As models CAN BE pretty heavy on the GPU memory there is 
// code to allow you to upload and unload them using the
// GPUModelPointer, which is basically a RAII over the 
// GPU allocated model. When there are no GPU users, the model
// is unloaded from the GPU.
// Models, once loaded, are always kept in RAM for fast uploading
struct GPUModelPointer
{
private:

	friend class Model;

	AssetHandle<Model> model;


public:

	GPUModelPointer(AssetHandle<Model> model);
	~GPUModelPointer();
};

template<>
class GenericSerializer<ModelConfig>
{
public:

	static void serialize(const ModelConfig& what, cpptoml::table& target)
	{
		logger->check(false, "Not implemented");
	}

	static void deserialize(ModelConfig& to, const cpptoml::table& from)
	{
		ModelConfig default = ModelConfig();

		SAFE_TOML_GET_OR(to.has_pos, "has_pos", bool, default.has_pos);
		SAFE_TOML_GET_OR(to.has_nrm, "has_nrm", bool, default.has_nrm);
		SAFE_TOML_GET_OR(to.has_nrm, "has_tex", bool, default.has_tex);
		SAFE_TOML_GET_OR(to.has_nrm, "has_tgt", bool, default.has_tgt);
		SAFE_TOML_GET_OR(to.has_nrm, "has_cl3", bool, default.has_cl3);
		SAFE_TOML_GET_OR(to.has_nrm, "has_cl4", bool, default.has_cl4);
		SAFE_TOML_GET_OR(to.has_nrm, "flip_uv", bool, default.flip_uv);
	}
};
