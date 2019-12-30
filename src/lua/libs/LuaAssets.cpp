#include "LuaAssets.h"
#include "../../assets/Image.h"

void LuaAssets::load_to(sol::table& table)
{
	table.new_usertype<LuaAssetHandle<Image>>("image_handle",
		"move", &LuaAssetHandle<Image>::move,
		"get", &LuaAssetHandle<Image>::get,
		sol::meta_function::to_string, [](const LuaAssetHandle<Image>& img)
	{
		return fmt::format("path: {}:{}, res: {}", img.pkg, img.name, (void*)img.data);
	},
		sol::meta_function::index, [](sol::usertype<LuaAssetHandle<Image>> self, sol::lua_value idx)
	{		
		auto img = self["get"](self).get<sol::usertype<Image>>();

		return img[idx].get<sol::object>();
	});


	table.set_function(
		"get_image", [](sol::this_state st, const std::string& path)
	{
		auto[pkg, name] = assets->get_package_and_name(path, sol::state_view(st)["__pkg"].get<std::string>());
		Image* img = assets->get<Image>(pkg, name, true);
		return std::move(LuaAssetHandle<Image>(pkg, name, img));
	});
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
}


template<typename T>
LuaAssetHandle<T>::LuaAssetHandle(const LuaAssetHandle<T>& p2)
{
	pkg = p2.pkg;
	name = p2.name;
	data = p2.data;
	ut = p2.ut;

	assets->get<T>(pkg, name, true);
}

template<typename T>
LuaAssetHandle<T>::LuaAssetHandle(const std::string& pkg, const std::string& name, T* data)
{
	this->pkg = pkg;
	this->name = name;
	this->data = data;
}

template<typename T>
LuaAssetHandle<T>::~LuaAssetHandle()
{
	if (data != nullptr)
	{
		assets->free<T>(pkg, name);
	}
}
