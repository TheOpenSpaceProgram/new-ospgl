#include "LuaAssets.h"
#include "../../assets/Image.h"

void LuaAssets::load_to(sol::table& table)
{
    // TODO: Fix on Windows
	table.new_usertype<LuaAssetHandle<Image>>("image_handle",
		"move", &LuaAssetHandle<Image>::move,
		"get", &LuaAssetHandle<Image>::get,
		sol::meta_function::to_string, [](const LuaAssetHandle<Image>& img)
	{
		return fmt::format("path: {}:{}, res: {}", img.pkg, img.name, (void*)img.data);
	});

	table.new_usertype<Image>("image",
		"get_size", &Image::get_sized,
		"get_width", &Image::get_width,
		"get_height", &Image::get_height,
		"sample_bilinear", sol::overload(
			sol::resolve<glm::dvec4(glm::dvec2)>(&Image::sample_bilinear_double),
			sol::resolve<glm::dvec4(double, double)>(&Image::sample_bilinear_double)
		)
		);

	table.set_function(
		"get_image", [](sol::this_state st, const std::string& path)
	{
		auto[pkg, name] = osp->assets->get_package_and_name(path, sol::state_view(st)["__pkg"].get<std::string>());
		Image* img = osp->assets->get<Image>(pkg, name, true);
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

	osp->assets->get<T>(pkg, name, true);
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
		osp->assets->free<T>(pkg, name);
	}
}
