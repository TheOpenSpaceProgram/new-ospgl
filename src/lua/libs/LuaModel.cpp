#include "LuaModel.h"
#include <assets/Model.h>

void LuaModel::load_to(sol::table &table)
{
	table.new_usertype<Node>("node", sol::no_constructor,
		  "draw", &Node::draw,
		  "draw_shadow", &Node::draw_shadow);

}
