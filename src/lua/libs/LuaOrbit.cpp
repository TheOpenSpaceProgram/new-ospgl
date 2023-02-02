#include "LuaOrbit.h"
#include <universe/predictor/QuickPredictor.h>
#include <universe/entity/trajectory/LandedTrajectory.h>

void LuaOrbit::load_to(sol::table &table)
{
	table.new_usertype<QuickPredictor>("quick_predictor",
	   "new", [](PlanetarySystem* sys)
			{
				return std::make_shared<QuickPredictor>(sys);
			},
			"get_drawer", &QuickPredictor::get_drawer,
			"update", &QuickPredictor::update,
			"launch", &QuickPredictor::launch);

	table.new_usertype<LandedTrajectory>("landed_trajectory", sol::no_constructor,
	 "new", [](const std::string& elem, glm::dvec3 rel_pos, glm::dquat rel_rot)
	 {
		 auto traj = std::make_shared<LandedTrajectory>();
		 traj->set_element(elem);
		 traj->initial_relative_pos = rel_pos;
		 traj->initial_rotation = rel_rot;
		 return traj;
	 },
	 "update", &LandedTrajectory::update);
}
