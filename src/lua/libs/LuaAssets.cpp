#include "LuaAssets.h"
#include "../../assets/BitmapFont.h"
#include "../../assets/BuildingPrototype.h"
#include "../../assets/Config.h"
#include "../../assets/Cubemap.h"
#include "../../assets/Image.h"
#include "../../assets/Material.h"
#include "../../assets/Model.h"
#include "../../assets/PartPrototype.h"
#include "../../assets/PhysicalMaterial.h"
#include "../../assets/Shader.h"

// Useful while development to make sure assets are working correctly
#define DEBUG_ENABLED

void LuaAssets::load_to(sol::table& table)
{
	// This huge macro is neccesary because lua doesn't allow templates, and there's no easy way to implement them
	#define IMPLEMENT_ASSET(cls, cls_name) \
	table.new_usertype<LuaAssetHandle<cls>>(cls_name "_handle", 												\
		"move", &LuaAssetHandle<cls>::move,                                             						\
		"get", &LuaAssetHandle<cls>::get,                                               						\
		sol::meta_function::to_string, [](const LuaAssetHandle<cls>& ast)              							\
		{                                                                               						\
		return fmt::format("Asset of type {}, path: {}:{}, pointer: {}", cls_name, ast.pkg, ast.name, (void*)ast.data);						\
	});                                   																	\
	table.set_function("get_" cls_name, [](sol::this_environment te, const std::string& path) 					\
	{                                      \
    	sol::environment& env = te;        																		\
    	auto[pkg, name] = osp->assets->get_package_and_name(path, env["__pkg"].get_or(std::string("core"))); 	\
        cls* ast = osp->assets->get<cls>(pkg, name, true);               \
        return std::move(LuaAssetHandle<cls>(pkg, name, ast));													\
	});                                    																		\

	IMPLEMENT_ASSET(BitmapFont, "bitmap_font");
	IMPLEMENT_ASSET(BuildingPrototype, "building_prototype");
	IMPLEMENT_ASSET(Config, "config");
	IMPLEMENT_ASSET(Cubemap, "cubemap");
	IMPLEMENT_ASSET(Image, "image");
	IMPLEMENT_ASSET(Material, "material");
	IMPLEMENT_ASSET(Model, "model");
	IMPLEMENT_ASSET(PartPrototype, "part_prototype");
	IMPLEMENT_ASSET(PhysicalMaterial, "physical_material");
	IMPLEMENT_ASSET(Shader, "shader");

	table.new_usertype<Image>("image",
		"get_size", &Image::get_sized,
		"get_width", &Image::get_width,
		"get_height", &Image::get_height,
		"sample_bilinear", sol::overload(
			sol::resolve<glm::dvec4(glm::dvec2)>(&Image::sample_bilinear_double),
			sol::resolve<glm::dvec4(double, double)>(&Image::sample_bilinear_double)
		)
		);

	table.set_function("get_udata_vehicle", [](const std::string& name)
	{
		logger->check(osp->assets->is_path_safe(name), "Path {} is unsafe and may access external files", name);
		std::string fix_name = name;
		// Remove start '/' as udata_path/vehicles/ already contains it
		while(fix_name[0] == '/')
		{
			fix_name = fix_name.substr(1);
		}
		std::string full_path = osp->assets->udata_path + "/vehicles/" + name;

		return SerializeUtil::load_file(full_path);
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

#ifdef DEBUG_ENABLED
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

#ifdef DEBUG_ENABLED
	logger->debug("Lua asset handle created (path={}:{}, pointer={}) by copy", pkg, name, (void*)data);
#endif
}

template<typename T>
LuaAssetHandle<T>::LuaAssetHandle(const std::string& pkg, const std::string& name, T* data)
{
	this->pkg = pkg;
	this->name = name;
	this->data = data;

#ifdef DEBUG_ENABLED
	logger->debug("Lua asset handle created (path={}:{}, pointer={})", pkg, name, (void*)data);
#endif
}

template<typename T>
LuaAssetHandle<T>::~LuaAssetHandle()
{
	if (data != nullptr)
	{
		logger->debug("Lua asset handle freed (path={}:{}, pointer={})", pkg, name, (void*)data);
		osp->assets->free<T>(pkg, name);
	}
}
