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
		"get_graphics_transform", [](Piece& self)
		{
			return BulletTransform(self.get_graphics_transform());
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
		"get_global_position", [](Piece& self)
		{
			return to_dvec3(self.get_global_transform().getOrigin());
		},
		"get_forward", &Piece::get_forward,
		"get_up", &Piece::get_up,
		"get_right", &Piece::get_right,
		"transform_axis", &Piece::transform_axis,
			
		"get_marker_position", &Piece::get_marker_position,
		"get_marker_rotation", &Piece::get_marker_rotation,
		"get_marker_transform", &Piece::get_marker_transform,
		"get_marker_forward", &Piece::get_marker_forward,
		"transform_point_to_rigidbody", &Piece::transform_point_to_rigidbody);

	table.new_usertype<Part>("part",
		"get_piece", &Part::get_piece);

	table.new_usertype<Machine>("machine",
		"init_toml", &Machine::init_toml,
		"add_output_port", &Machine::add_output_port,
		"add_input_port", &Machine::add_input_port,
		"write_to_port", sol::overload(
			[](Machine& self, const std::string& name, double v)
			{
				self.write_to_port(name, PortValue(v));
			}
		)
	);

	table.new_usertype<PortResult>("port_result",
		"good", [](PortResult& self)
		{
			return self.result == PortResult::GOOD;
		},
		"failed", [](PortResult& self)
		{
			return self.result != PortResult::GOOD;
		},
		"result", &PortResult::result
		);

	table.new_enum("port_result_type",
		"GOOD", PortResult::GOOD,
		"INVALID_TYPE", PortResult::INVALID_TYPE,
		"PORT_BLOCKED", PortResult::PORT_BLOCKED,
		"PORT_NOT_FOUND", PortResult::PORT_NOT_FOUND);
}
