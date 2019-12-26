#pragma once
#include <glad/glad.h>
#include "AssetManager.h"
#include "Shader.h"
#include "Material.h"
#include "../renderer/camera/CameraUniforms.h"

struct aiScene;
struct aiNode;
struct aiMesh;



class Mesh 
{
private:

	friend class Model;

	GLuint vbo, vao, ebo;

	size_t data_size;
	size_t index_count;

	float* data;
	uint32_t* indices;

	// It's only loaded while we are uploaded
	AssetHandle<Material> material;

	std::vector<AssimpTexture> textures;

	bool drawable;

public:

	// Stuff that starts with 'col_' are colliders
	// Any children of a non-drawable is not drawable!
	bool is_drawable();

	void upload();
	void unload();

	// Binds core uniforms and material uniforms
	void bind_uniforms(const CameraUniforms& uniforms, glm::dmat4 model);

	// Only issues the draw command, does absolutely nothing else
	void draw_command();

	// We take ownership of the handle
	Mesh(AssetHandle<Material>&& mat) : material(std::move(mat))
	{
		vao = 0;
		vbo = 0;
		ebo = 0;
		data = nullptr;
		indices = nullptr;
	}

};

struct Node
{
	std::string name;

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

	void process_node(aiNode* node, const aiScene* scene, Node* to, bool drawable);
	void process_mesh(aiMesh* mesh, const aiScene* scene, Node* to, bool drawable);


	friend class Mesh;
	friend struct GPUModelPointer;

	int gpu_users;

	bool uploaded;

	Node* root;

	void upload();
	void unload();

public:

	std::unordered_map<std::string, Node*> node_by_name;

	void get_gpu();
	void free_gpu();

	Model(const aiScene* scene);
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

	// You could store the Node pointer, but make sure
	// it doesn't outlive the GPUModelPointer.
	// It's better to simply call this every time, it's
	// pretty fast!
	Node* get_node(std::string name);

	// Same as above
	Node* get_root_node();

	// We take ownership of the asset handle (std::move)
	GPUModelPointer(AssetHandle<Model>&& model);
	~GPUModelPointer();
};

