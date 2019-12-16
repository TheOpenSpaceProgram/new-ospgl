#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


void Model::process_node(aiNode* node, const aiScene* scene, Node* to)
{
	Node* n_node = new Node();

	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		process_mesh(mesh, scene, n_node);
	}

	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		process_node(node->mChildren[i], scene, n_node);
	}

	if (to == nullptr)
	{
		root = n_node;
	}
	else
	{
		to->children.push_back(n_node);
	}

}

void Model::process_mesh(aiMesh* mesh, const aiScene* scene, Node* to)
{
	to->meshes.push_back(Mesh(config));
	Mesh* m = &to->meshes[to->meshes.size() - 1];

	size_t vert_size = config.get_vertex_size();
	size_t floats_per_vert = config.get_vertex_floats();

	// Work directly in the vector
	m->data = (float*)malloc(vert_size * mesh->mNumVertices);
	m->indices = (uint32_t*)malloc(sizeof(uint32_t) * mesh->mNumFaces * 3);

	bool log_once_tangents = false;

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		m->indices[i * 3 + 0] = face.mIndices[0];
		m->indices[i * 3 + 1] = face.mIndices[1];
		m->indices[i * 3 + 2] = face.mIndices[2];
	}

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		size_t i0 = i * floats_per_vert;
		constexpr size_t o = 1; //< 1 Because we index using floats, could be removed ;)
		size_t j = 0;

		if (config.has_pos)
		{
			m->data[i0 + j * o] = mesh->mVertices[i].x; j++;
			m->data[i0 + j * o] = mesh->mVertices[i].y; j++;
			m->data[i0 + j * o] = mesh->mVertices[i].z; j++;
		}

		if (config.has_nrm)
		{
			m->data[i0 + j * o] = mesh->mNormals[i].x; j++;
			m->data[i0 + j * o] = mesh->mNormals[i].y; j++;
			m->data[i0 + j * o] = mesh->mNormals[i].z; j++;
		}

		if (config.has_tex)
		{
			if (mesh->mTextureCoords[0])
			{
				// We only support one UV map
				m->data[i0 + j * o] = mesh->mTextureCoords[0][i].x; j++;
				m->data[i0 + j * o] = mesh->mTextureCoords[0][i].y; j++;
			}
			else
			{
				m->data[i0 + j * o] = 0.0f; j++;
				m->data[i0 + j * o] = 0.0f; j++;
			}
			
		}

		if (config.has_tgt)
		{
			if (mesh->mTangents)
			{
				m->data[i0 + j * o] = mesh->mTangents[i].x; j++;
				m->data[i0 + j * o] = mesh->mTangents[i].y; j++;
				m->data[i0 + j * o] = mesh->mTangents[i].z; j++;
				m->data[i0 + j * o] = mesh->mBitangents[i].x; j++;
				m->data[i0 + j * o] = mesh->mBitangents[i].y; j++;
				m->data[i0 + j * o] = mesh->mBitangents[i].z; j++;
			}
			else
			{
				m->data[i0 + j * o] = 0.0f; j++;
				m->data[i0 + j * o] = 0.0f; j++;
				m->data[i0 + j * o] = 0.0f; j++;
				m->data[i0 + j * o] = 0.0f; j++;
				m->data[i0 + j * o] = 0.0f; j++;
				m->data[i0 + j * o] = 0.0f; j++;

				if (!log_once_tangents)
				{
					logger->warn("Could not find tangents. Make sure your model has texture coordinates and normals!");
					log_once_tangents = true;
				}
			}
			
		}

		if (config.has_cl3)
		{
			// We only support one color map
			m->data[i0 + j * o] = mesh->mColors[0][i].r; j++;
			m->data[i0 + j * o] = mesh->mColors[0][i].g; j++;
			m->data[i0 + j * o] = mesh->mColors[0][i].b; j++;
		}

		if (config.has_cl4)
		{
			m->data[i0 + j * o] = mesh->mColors[0][i].r; j++;
			m->data[i0 + j * o] = mesh->mColors[0][i].g; j++;
			m->data[i0 + j * o] = mesh->mColors[0][i].b; j++;
			m->data[i0 + j * o] = mesh->mColors[0][i].a; j++;
		}
	}
}


void Mesh::upload()
{
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);
	glGenVertexArrays(1, &vao);

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, data_size, data, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(uint32_t), indices, GL_STATIC_DRAW);

	size_t ptr_num = 0;
	size_t stride = cfg.get_vertex_floats();
	size_t offset = 0;

	if (cfg.has_pos)
	{
		// Position
		glEnableVertexAttribArray(ptr_num);
		glVertexAttribPointer(ptr_num, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);

		ptr_num++;
		offset += 3;
	}

	if (cfg.has_nrm)
	{
		// Normal
		glEnableVertexAttribArray(ptr_num);
		glVertexAttribPointer(ptr_num, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);

		ptr_num++;
		offset += 3;
	}

	if (cfg.has_tex)
	{
		// Texture 
		glEnableVertexAttribArray(ptr_num);
		glVertexAttribPointer(ptr_num, 2, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);

		ptr_num++;
		offset += 2;
	}

	if (cfg.has_tgt)
	{
		// Tangent
		glEnableVertexAttribArray(ptr_num);
		glVertexAttribPointer(ptr_num, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);
		ptr_num++;
		offset += 3;

		// Bitangent
		glEnableVertexAttribArray(ptr_num);
		glVertexAttribPointer(ptr_num, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);
		ptr_num++;
		offset += 3;
	}

	if (cfg.has_cl3)
	{
		// Color (RGB)
		glEnableVertexAttribArray(ptr_num);
		glVertexAttribPointer(ptr_num, 3, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);

		ptr_num++;
		offset += 3;
	}

	if (cfg.has_cl4)
	{
		// Color (RGBA)
		glEnableVertexAttribArray(ptr_num);
		glVertexAttribPointer(ptr_num, 4, GL_FLOAT, GL_FALSE, stride * sizeof(float), (void*)offset);

		ptr_num++;
		offset += 4;
	}
	
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::unload()
{
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);
	glDeleteVertexArrays(1, &vao);

	vao = 0;
	ebo = 0;
	vbo = 0;
}

void Mesh::draw()
{
	logger->check(vao != 0, "Tried to render a non-loaded mesh");

	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, (GLsizei)index_count, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

}


void Model::upload()
{

	
	uploaded = true;
}

void Model::unload()
{
	uploaded = false;
}

void Model::get_gpu()
{
	gpu_users++;

	if (!uploaded)
	{
		upload();
	}
}

void Model::free_gpu()
{
	gpu_users--;
	if (gpu_users <= 0)
	{
		unload();
	}
}



Model::Model(ModelConfig config, const aiScene* scene)
{
	this->config = config;
	gpu_users = 0;
	uploaded = false;

	process_node(scene->mRootNode, scene, nullptr);
}

Model::~Model()
{
}

Model* loadModel(const std::string& path, const std::string& name, const std::string& pkg, const cpptoml::table& cfg)
{
	ModelConfig config = ModelConfig();

	if (cfg.get_table_qualified("model"))
	{
		auto ctoml = cfg.get_table_qualified("model");
		SerializeUtil::read_to(*ctoml, config);
	}

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, config.get_assimp_flags());

	Model* m = new Model(config, scene);

	return m;
}


GPUModelPointer::GPUModelPointer(AssetHandle<Model> model)
{
	model->get_gpu();
}

GPUModelPointer::~GPUModelPointer()
{
	model->free_gpu();
}

ModelConfig::ModelConfig()
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

size_t ModelConfig::get_vertex_floats()
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

size_t ModelConfig::get_vertex_size()
{
	return get_vertex_floats() * sizeof(float);
}

unsigned int ModelConfig::get_assimp_flags()
{
	unsigned int flags = aiProcess_Triangulate | aiProcess_JoinIdenticalVertices;

	if (has_nrm)
	{
		flags |= aiProcess_GenSmoothNormals;
	}

	if (has_tex)
	{
		flags |= aiProcess_GenUVCoords;
	}

	if (flip_uv)
	{
		flags |= aiProcess_FlipUVs;
	}

	if (has_tgt)
	{
		flags |= aiProcess_CalcTangentSpace;
	}

	// We don't allow lines or points so this allows us to easily erase them
	flags |= aiProcess_SortByPType;

	return flags;
}
