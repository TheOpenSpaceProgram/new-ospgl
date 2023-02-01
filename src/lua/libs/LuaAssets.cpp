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

void LuaAssets::load_to(sol::table& table)
{
	// This huge macro is neccesary because lua doesn't allow templates, and there's no easy way to implement them
	#define IMPLEMENT_ASSET(cls, cls_name) \
	table.new_usertype<LuaAssetHandle<cls>>(cls_name "_handle", 												\
		"move", &LuaAssetHandle<cls>::move,                                             						\
		"get", &LuaAssetHandle<cls>::get,     \
        "get_asset_id", &LuaAssetHandle<cls>::get_asset_id,                                \
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

	table.new_usertype<Config>("config",
		"root", &Config::root,
		// This should work except in the most strange of cases
		"push_pkg", [](Config* cfg, sol::this_environment tenv)
	    {
			sol::environment env = tenv;
			if(env["__pkg_stack"] == sol::nil)
			{
				env["__pkg_stack"] = sol::new_table();
			}
			auto table = env["__pkg_stack"].get<sol::table>();
			// Push into the stack
			table[table.size()] = env["__pkg"];
			env["__pkg"] = cfg->get_asset_pkg();
	    },
		"restore_pkg", [](Config* cfg, sol::this_environment tenv)
		{
			sol::environment  env = tenv;
			auto table = env["__pkg_stack"].get<sol::table>();
			// Pop from the table
			env["__pkg"] = table[table.size()];
			table[table.size()] = sol::nil;
		});

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

