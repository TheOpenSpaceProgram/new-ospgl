#pragma once
#include <glad/glad.h>
#include "AssetManager.h"
#include "Shader.h"
#include "Material.h"
#include <renderer/camera/CameraUniforms.h>
#include <renderer/lighting/ShadowCamera.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <physics/glm/BulletGlmCompat.h>


struct aiScene;
struct aiNode;
struct aiMesh;

class Mesh 
{
private:


	GLuint vbo, vao, ebo;



	// It's only loaded while we are uploaded
	AssetHandle<Material> material;

	std::vector<AssimpTexture> textures;

	bool drawable;

public:

	friend class Model;
	friend struct Node;

	size_t data_size;
	size_t index_count;

	// Only present on non drawable meshes! Used internally
	std::vector<glm::vec3> verts;

	// data is only present on drawable meshes
	float* data;

	// Present on both, as both use indexed vertices
	uint32_t* indices;

	// Stuff that starts with 'col_' are colliders
	// Any children of a non-drawable is not drawable!
	// Non-drawable stuff gets the aditional vertex positions loaded
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
	glm::dmat4 sub_transform;

	std::vector<Node*> children;

	std::unordered_map<std::string, std::string> properties;

	void draw_all_meshes(const CameraUniforms& uniforms, glm::dmat4 model);
	void draw_all_meshes_shadow(const ShadowCamera& sh_cam, glm::dmat4 model);

	// Draws all meshes, and call sthe same on all children,
	// accumulating sub transforms
	// The ignore_our_subtform flag is useful specially for stuff like parts
	// where the piece transform is ignored during game rendering
	void draw(const CameraUniforms& uniforms, glm::dmat4 model, bool ignore_our_subtform = false);
	void draw_shadow(const ShadowCamera& sh_cam, glm::dmat4 model, bool ignore_our_subtform = false);
};

// Models allow loading 3d models using the assimp library
// For now we support (and have tested):
// - .fbx (Easy to export from blender, make sure custom-properties are enabled for collision to work!)
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

	static constexpr const char* COLLIDER_PREFIX = "col_";
	static constexpr const char* MARK_PREFIX = "m_";

	std::unordered_map<std::string, Node*> node_by_name;

	void get_gpu();
	void free_gpu();

	Model(const aiScene* scene);
	~Model();
};

Model* load_model(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg);

// As models CAN BE pretty heavy on the GPU memory there is 
// code to allow you to upload and unload them using the
// GPUModelPointer, which is basically a RAII over the 
// GPU allocated model. When there are no GPU users, the model
// is unloaded from the GPU.
// Models, once loaded, are always kept in RAM for fast uploading
// They are a non-copyable resource, similar to AssetHandle
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

	GPUModelPointer() : model()
	{

	}

	// We must take the ownership
	GPUModelPointer(GPUModelPointer&& b)
	{
		this->model = std::move(b.model);
		b.model = AssetHandle<Model>();
	}

	GPUModelPointer& operator=(GPUModelPointer&& b)
	{
		this->model = std::move(b.model);
		b.model = AssetHandle<Model>();
		return *this;
	}

	GPUModelPointer duplicate() const
	{
		return GPUModelPointer(model.duplicate());
	}

	bool is_null()
	{
		return model.is_null();
	}

	// We take ownership of the asset handle (std::move)
	GPUModelPointer(AssetHandle<Model>&& model);
	~GPUModelPointer();

	Model* operator->()
	{
		return model.operator->();
	}

	Model& operator*()
	{
		return model.operator*();
	}

};

// Useful if you only want a node, same as before, non copyable
struct GPUModelNodePointer
{
	GPUModelPointer model_ptr;
	Node* sub_node;

	GPUModelNodePointer() : model_ptr()
	{
		sub_node = nullptr;
	}

	GPUModelNodePointer(GPUModelNodePointer&& b) : model_ptr(std::move(b.model_ptr))
	{
		b.model_ptr = GPUModelPointer(AssetHandle<Model>());
		sub_node = b.sub_node;
	}

	GPUModelNodePointer& operator=(GPUModelNodePointer&& b)
	{
		this->model_ptr = std::move(b.model_ptr);
		b.model_ptr = GPUModelPointer(AssetHandle<Model>());
		this->sub_node = b.sub_node;

		return *this;
	}

	GPUModelNodePointer duplicate() const
	{
		return GPUModelNodePointer(std::move(model_ptr.duplicate()), sub_node->name);
	}

	bool is_null()
	{
		return model_ptr.is_null();
	}

	GPUModelNodePointer(GPUModelPointer&& n_model_ptr, const std::string& sub_name) : model_ptr(std::move(n_model_ptr))
	{
		this->sub_node = model_ptr.get_node(sub_name);
	}

	// Automatic destructor is good


	Node* operator->()
	{
		return sub_node;
	}

	Node& operator*()
	{
		return *sub_node;
	}


};


// Used to easily extract colliders from any model
class ModelColliderExtractor
{
public:

	// Simple checks
	static void single_collider_common(Node* n);

	//        min        max
	static std::pair<glm::vec3, glm::vec3> obtain_bounds(Mesh* m);

	static void load_collider_compound(btCollisionShape** target, Node* n);
	static void load_collider_box(btCollisionShape** target, Node* n);
	static void load_collider_sphere(btCollisionShape** target, Node* n);
	static void load_collider_cylinder(btCollisionShape** target, Node* n);
	static void load_collider_cone(btCollisionShape** target, Node* n);
	static void load_collider_capsule(btCollisionShape** target, Node* n);
	static void load_collider_concave(btCollisionShape** target, Node* n);
	static void load_collider_convex(btCollisionShape** target, Node* n);

	static void load_collider(btCollisionShape** target, Node* n);
};
