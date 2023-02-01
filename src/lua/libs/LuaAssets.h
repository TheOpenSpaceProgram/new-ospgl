#pragma once
#include "../LuaLib.h"
#include <util/Logger.h>
#include <assets/AssetManager.h>


// Useful while development to make sure assets are working correctly
#define LUA_ASSET_DEBUG_ENABLED

/*
	Lua functions to interact with the AssetManager subsystem,
	allows loading of resource types which have a lua lib:

 	Note: You must also load the pertinent library for handling said asset,
 	this library only provides means to use the asset handles
	
	Paths are always given as package paths, we don't allow arbitrary
	path access for safety.

		- get_image(path, load_now = true)

	All functions return a LuaAssetHandle, if the resource could not be loaded,
	trying to get the resource will return nil

		- get_string(path)

	Does not return a LuaAssetHandle, instead returns the string directly (or nil).

	LuaAssetHandle ("asset_handle"):

		As resources are usually quite heavy, we need a way to manage memory.
		The C++ AssetManager uses AssetHandles for this, but templates are not
		supported in lua so we instead use a custom implementation.

		You can access the inner resource by calling :get, but make sure that you
		don't store the return for long as the asset may be unloaded and you will
		get garbage (or crash the program).

		You cannot create your own asset handles, you must obtain them from get_x

		You can use asset:move() to invalidate a asset handle, returning a new valid one

	Note: This loads all asset types properly (TODO: It doesn't, implement it...)

	Note to coders:
		
		When implementing an asset type, you should receive LuaAssetHandle<X>& instead of X as
		self, this is a limitation that seems hard to get around. (Typecasting overloading does not seem 
		to work well with sol)

		When a resource takes a LuaAssetHandle, it should be duplicated, not moved as usual in C++ code
		(This is because in lua you can simply do resource = nil, but there's no move semantics!)

 	-> Furthermore, it allows reading / writing vehicle TOML files from udata

*/
class LuaAssets : public LuaLib
{
public:

	virtual void load_to(sol::table& table) override;
};

// Note to C++ programmers (and TODO if possible):
// LuaAssetHandles for some reason cannot be casted to AssetHandles if passed by reference!
// Implement a wrapper function or use pointers (better to use a wrapper for consistency in C++ side)
template<typename T>
struct LuaAssetHandle
{
	// Used when getting assets from lua where .data cannot be used
	AssetHandle<T> get_asset_handle() const
	{
		if(data == nullptr)
		{
			return AssetHandle<T>();
		}

		return AssetHandle<T>(pkg, name);
	}


	std::string pkg;
	std::string name;

	T* data;
	sol::usertype<T> ut;

	T* operator()()
	{
		return data;
	}

	T* operator->()
	{
		return data;
	}

	std::string get_asset_id()
	{
		return pkg + ":" + name;
	}

	T* get()
	{
		return data;
	}

	explicit operator T()
	{
		return *data;
	}

	// Invalidates this (makes it null) and returns a new valid handle
	// Same as the move constructor but for lua
	LuaAssetHandle<T> move();

	// Move constructor
	LuaAssetHandle(LuaAssetHandle<T>&& b);
	// Copy constructor
	LuaAssetHandle(const LuaAssetHandle<T> &p2);

	LuaAssetHandle(const std::string& pkg, const std::string& name, T* data);
	// This "duplicates" the passed asset handle
	LuaAssetHandle(const AssetHandle<T>& from);
	~LuaAssetHandle();

	bool operator==(const LuaAssetHandle<T>& other) const
	{
		return pkg == other.pkg && name == other.name;
	}
};

template<typename T>
LuaAssetHandle<T>::LuaAssetHandle(const AssetHandle<T>& from)
{
	pkg = from.pkg;
	name = from.name;
	data = from.data;

	osp->assets->get<T>(pkg, name, true);

#ifdef LUA_ASSET_DEBUG_ENABLED
	logger->debug("Lua asset handle created (path={}:{}, pointer={}) by copy from asset handle", pkg, name, (void*)data);
#endif
}

template<typename T>
LuaAssetHandle<T> LuaAssetHandle<T>::move()
{
	return LuaAssetHandle<T>(std::move(*this));
}

template<typename T>
LuaAssetHandle<T>::LuaAssetHandle(LuaAssetHandle<T>&& b)
{
	pkg = b.pkg;
	name = b.name;
	data = b.data;
	ut = b.ut;

	b.pkg = "null";
	b.name = "null";
	b.data = nullptr;
	b.ut = sol::nil;

#ifdef LUA_ASSET_DEBUG_ENABLED
	logger->debug("Lua asset handle created (path={}:{}, pointer={}) by move", pkg, name, (void*)data);
#endif
}


template<typename T>
LuaAssetHandle<T>::LuaAssetHandle(const LuaAssetHandle<T>& p2)
{
	pkg = p2.pkg;
	name = p2.name;
	data = p2.data;
	ut = p2.ut;

	osp->assets->get<T>(pkg, name, true);

#ifdef LUA_ASSET_DEBUG_ENABLED
	logger->debug("Lua asset handle created (path={}:{}, pointer={}) by copy", pkg, name, (void*)data);
#endif
}

template<typename T>
LuaAssetHandle<T>::LuaAssetHandle(const std::string& pkg, const std::string& name, T* data)
{
	this->pkg = pkg;
	this->name = name;
	this->data = data;

#ifdef LUA_ASSET_DEBUG_ENABLED
	logger->debug("Lua asset handle created (path={}:{}, pointer={})", pkg, name, (void*)data);
#endif
}


template<typename T>
LuaAssetHandle<T>::~LuaAssetHandle()
{
	if (data != nullptr)
	{
#ifdef LUA_ASSET_DEBUG_ENABLED
		logger->debug("Lua asset handle freed (path={}:{}, pointer={})", pkg, name, (void*)data);
#endif
		osp->assets->free<T>(pkg, name);
	}
}
