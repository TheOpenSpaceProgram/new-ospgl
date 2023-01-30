#include "LuaOrbit.h"
#include <universe/predictor/QuickPredictor.h>

void LuaOrbit::load_to(sol::table &table)
{
	table.new_usertype<QuickPredictor>("quick_predictor",
	   "new", [](PlanetarySystem* sys)
			{
				return std::make_shared<QuickPredictor>(sys);
			},
			"get_drawer", &QuickPredictor::get_drawer,
			"launch", &QuickPredictor::launch);

}
