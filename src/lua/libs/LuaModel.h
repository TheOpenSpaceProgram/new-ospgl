#pragma once
#include "../LuaLib.h"
#include "LuaAssets.h"
#include "assets/Model.h"

class LuaModel : public LuaLib
{
public:

	void load_to(sol::table& table) override;

};

// Allows usage of GPUModelPointers from lua with a reasonable syntax
// Holds an AssetHandle, so no need to store that separately
struct LuaGPUModelPointer
{
	LuaAssetHandle<Model> model_handle;

	// You are guaranteed that these nodes are on the GPU and thus are drawable
	Node* get_node(const std::string& name);
	Node* get_root();

	// Invalidates us and returns a new pointer
	LuaGPUModelPointer move();

	// Move constructor from LuaAssetHandle
	LuaGPUModelPointer(LuaAssetHandle<Model>&& h) : model_handle(h){}
	// Move constructor
	LuaGPUModelPointer(LuaGPUModelPointer &&b) : model_handle(std::move(b.model_handle)) {}
	// Copy constructor
	LuaGPUModelPointer(const LuaGPUModelPointer &p2) : model_handle(p2.model_handle) {}

	bool operator==(const LuaGPUModelPointer& other) const
	{
		return other.model_handle.data == model_handle.data;
	}

	~LuaGPUModelPointer();
};


