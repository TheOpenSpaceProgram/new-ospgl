#include "LuaVehicle.h"
#include "../../vehicle/Vehicle.h"

void LuaVehicle::load_to(sol::table& table)
{
	table.new_usertype<Vehicle>("vehicle");	

	table.new_usertype<Piece>("piece",
		"rigid_body", &Piece::rigid_body,
		"welded", &Piece::welded,
		"get_global_transform", [](Piece& self)
		{
			return BulletTransform(self.get_global_transform());
		},
		"get_local_transform", [](Piece& self)
		{
			return BulletTransform(self.get_local_transform());
		},
		"get_linear_velocity", [](Piece& self)
		{
			return to_dvec3(self.get_linear_velocity());
		},
		"get_angular_velocity", [](Piece& self)
		{
			return to_dvec3(self.get_angular_velocity());
		},
		"get_tangential_velocity", [](Piece& self)
		{
			return to_dvec3(self.get_tangential_velocity());
		},
		"get_relative_position", [](Piece& self)
		{
			return to_dvec3(self.get_relative_position());
		},
		"get_forward", &Piece::get_forward,
		"get_up", &Piece::get_up,
		"get_right", &Piece::get_right,
		"transform_axis", &Piece::transform_axis);

	table.new_usertype<Part>("part",
		"get_piece", &Part::get_piece);

	table.new_usertype<Machine>("machine",
		"init_toml", &Machine::init_toml,
		"add_output_port", [](Machine& self, const std::string& name, const std::string& type)
		{
			self.add_port(name, type, true);	
		},
		"add_input_port", [](Machine& self, const std::string& name, const std::string& type)
		{
			self.add_port(name, type, false);
		}
	);
}
