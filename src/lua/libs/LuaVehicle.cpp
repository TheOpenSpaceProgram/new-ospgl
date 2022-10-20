#include "LuaVehicle.h"
#include "../../universe/vehicle/Vehicle.h"

static WorldState decode_worldstate_table(sol::table table)
{
	WorldState out;
	out.cartesian.pos = table["pos"].get<glm::dvec3>();
	out.cartesian.vel = table["vel"].get<glm::dvec3>();
	out.rotation = table["rot"].get<glm::dquat>();
	glm::dvec3 v = glm::dvec3(0, 0, 0);
	out.angular_velocity = table["angvel"].get_or<glm::dvec3>(v);

	return out;
}


void LuaVehicle::load_to(sol::table& table)
{
	// Note to coders: Vehicle is deallocated manually so we can return a raw pointer fine
	table.new_usertype<Vehicle>("vehicle",
		 "new", [](){return new Vehicle();},
		 	"is_packed", &Vehicle::is_packed,
		 	"packed", &Vehicle::packed_veh,
		 	"unpacked", &Vehicle::unpacked_veh,
		 	"unpack", &Vehicle::unpack,
		 	"all_pieces", &Vehicle::all_pieces,
		 	"root", &Vehicle::root,
		 	"parts", &Vehicle::parts,
		 	"update", &Vehicle::update,
		 	"remove_piece", &Vehicle::remove_piece,
		 	"physics_update", &Vehicle::physics_update,
		 	"init", sol::resolve<void(Universe*, Entity*)>(&Vehicle::init),
		 	"update_attachments", &Vehicle::update_attachments,
		 	"set_world", sol::resolve<void(btDiscreteDynamicsWorld*)>(&Vehicle::set_world),
		 	"sort", &Vehicle::sort,
		 	"remove_outdated", &Vehicle::remove_outdated,
		 	"get_children_of", &Vehicle::get_children_of,
		 	"get_attached_to", &Vehicle::get_attached_to,
		 	"get_connected_to", &Vehicle::get_connected_to,
		 	"get_connected_with", &Vehicle::get_connected_with);//,
		//"get_attached_to", sol::overload(
		//	sol::resolve<std::vector<Piece*>(Piece*)>(Vehicle::get_attached_to),
		//	sol::resolve<Piece*(Piece*, const std::string&)>(Vehicle::get_attached_to)
		//));

	table.new_usertype<PackedVehicle>("packed_vehicle", sol::no_constructor,
	      "vehicle", sol::readonly(&PackedVehicle::vehicle),
	      "set_world_state", [](PackedVehicle* self, sol::table wstate)
		  {
				self->set_world_state(decode_worldstate_table(wstate));
		  });

	table.new_usertype<UnpackedVehicle>("unpacked_vehicle", sol::no_constructor,
		 "get_center_of_mass", &UnpackedVehicle::get_center_of_mass);

	table.new_usertype<Piece>("piece",
		"rigid_body", &Piece::rigid_body,
		"welded", &Piece::welded,
		"attached_to", &Piece::attached_to,
		"set_dirty", &Piece::set_dirty, 
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
		"get_graphics_position", [](Piece& self)
		{
			return to_dvec3(self.get_graphics_transform().getOrigin());
		},
		"get_forward", &Piece::get_forward,
		"get_up", &Piece::get_up,
		"get_right", &Piece::get_right,
		"transform_axis", &Piece::transform_axis,
			
		"get_marker_position", &Piece::get_marker_position,
		"get_marker_rotation", &Piece::get_marker_rotation,
		"get_marker_transform", &Piece::get_marker_transform,
		"get_marker_forward", &Piece::get_marker_forward,
		"transform_point_to_rigidbody", &Piece::transform_point_to_rigidbody,
		"get_attached_to_this", [](Piece& self)
		{
			return self.in_vehicle->get_attached_to(&self);
		},
		"get_attached_to_marker", [](Piece& self, const std::string& marker)
		{
			return self.in_vehicle->get_connected_with(&self, marker);
		},
		"get_environment_pressure", &Piece::get_environment_pressure,
		"collider_offset", &Piece::collider_offset,
		// Do not hold the returned value! for inmediate calling only!
		"get_model_node", [](Piece* p)
		{
			return p->model_node.sub_node;
		});

	table.new_usertype<Part>("part",
		"id", &Part::id,
		"get_piece", &Part::get_piece,
		"get_machine", &Part::get_machine);

	table.new_usertype<Machine>("machine",
		"runtime_uid", &Machine::runtime_uid,
		"init_toml", &Machine::init_toml,
		"plumbing", &Machine::plumbing,
		"in_part", &Machine::in_part,
		"load_interface", [](Machine* self, const std::string& iname, sol::this_state tst, sol::this_environment tenv)
		{
			sol::environment old_env = tenv;
			sol::state_view sv = tst;
			sol::environment env = sol::environment(sv, sol::create, sv.globals());
			// We also add the special function 'create_interface' to the environment
			// (Yep this is a nested lambda)
			// TODO: This code may be unneccesary? Think of it
			env["create_interface"] = [self](sol::this_state tst)
			{
				sol::state_view sv = tst;
				sol::table out = sv.create_table();

				out["machine"] = self;

				return out;
			};

			auto[pkg, name] = osp->assets->get_package_and_name(iname, old_env["__pkg"]);
			std::string sane_name = pkg + ":" + name;
			std::string script = osp->assets->load_string(sane_name);
			sol::table n_table = sv.script(script, env, (const std::string&) sane_name).get<sol::table>();
			self->load_interface(sane_name, n_table);
			return n_table;
		},
		"get_all_wired_machines", sol::overload([](Machine& self)
		{
			return self.get_all_wired_machines();
		},
		[](Machine& self, bool include_this)
		{
			return self.get_all_wired_machines(include_this);
		}),
		"get_wired_machines_with", [](Machine& self, sol::variadic_args args)
		{
			// We build the vector, something which sol cannot do automatically
			// and also find any 'false' value which sets include_this to false
			std::vector<std::string> vec;
			bool include_this = true;
			for(auto arg : args)
			{
				if(arg.get_type() == sol::type::boolean)
				{
					if(arg.get<bool>() == false)
					{
						include_this = false;
					}
				}
				else if(arg.get_type() == sol::type::string)
				{
					vec.push_back(arg.get<std::string>());
				}
			}

			return self.get_wired_machines_with(vec, include_this);
		},
		"get_wired_interfaces", 
		sol::overload([](Machine& self, const std::string& int_type, bool include_this)
		{
			return self.get_wired_interfaces(int_type, include_this);
		},
		[](Machine& self, const std::string& int_type)
		{
			return self.get_wired_interfaces(int_type);
		}),
		"get_interface", &Machine::get_interface
		
	);


}
