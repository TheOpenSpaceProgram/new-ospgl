#include "LuaGameDatabase.h"
#include <game/database/GameDatabase.h>

void LuaGameDatabase::load_to(sol::table& table)
{
	table.new_usertype<GameDatabase>("game_database",
	"add_part", [](GameDatabase* self, const std::string& path, sol::this_environment st)
	{
		sol::environment view = st;
		std::string pkg = view["__pkg"];
		self->add_part(path, pkg);
	},
	"add_part_category", [](GameDatabase* self, const std::string& path, sol::this_environment st)
	{
		sol::environment view = st;
		std::string pkg = view["__pkg"];
		self->add_part_category(path, pkg);
	},
	"add_material", [](GameDatabase* self, const std::string& path, sol::this_environment st)
	{
		sol::environment view = st;
		std::string pkg = view["__pkg"];
		self->add_material(path, pkg);
	});
}
