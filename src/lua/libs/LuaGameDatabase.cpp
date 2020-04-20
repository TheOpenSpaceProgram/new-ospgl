#include "LuaGameDatabase.h"
#include <game/database/GameDatabase.h>

void LuaGameDatabase::load_to(sol::table& table)
{
	table.new_usertype<GameDatabase>("game_database",
	"add_part", [](GameDatabase* self, const std::string& path, sol::this_state st)
	{
		sol::state_view view = sol::state_view(st);
		std::string pkg = view["__pkg"];
		self->add_part(path, pkg);
	},
	"add_part_category", [](GameDatabase* self, const std::string& path, sol::this_state st)
	{
		sol::state_view view = sol::state_view(st);
		std::string pkg = view["__pkg"];
		self->add_part_category(path, pkg);
	});
}
