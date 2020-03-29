local bullet = require("bullet");
local glm = require("glm");

local constraint = nil;
local bt_world = nil;

function load_toml(toml_data)

end

function activate(from_rigid, from_frame, to_rigid, to_frame, world)
	if constraint == nil then 
		bt_world = world

		constraint = bullet.generic_6dof_spring_constraint.new(from_rigid, to_rigid, from_frame, to_frame, true)
		constraint:enable_spring(2, true)
		constraint:set_stiffness(2, 1000000.0)
		constraint:set_damping(2, 0.00001)
		constraint:set_linear_upper_limit(glm.vec3.new(0.04, 0.04, 0.5))
		constraint:set_linear_lower_limit(glm.vec3.new(-0.04, -0.04, -0.5))
		constraint:set_angular_upper_limit(glm.vec3.new(0, 0, 1.5))
		constraint:set_angular_lower_limit(glm.vec3.new(0, 0, -1.5))
		constraint:set_debug_draw_size(5.0)
		constraint:set_equilibrium_point()

		constraint:enable_spring(0, true)
		constraint:set_stiffness(0, 100000.0)
		constraint:set_damping(0, 0.00001)
		
		constraint:enable_spring(1, true)
		constraint:set_stiffness(1, 100000.0)
		constraint:set_damping(1, 0.00001)


		constraint:add_to_world(world, true)
	end
end

function deactivate()	
	if constraint ~= nil then 
		constraint:remove_from_world(bt_world)
		bt_world = nil
		constraint = nil
	end

end 

function is_broken() 

	if constraint == nil then
		return true 
	else
		return not constraint:is_enabled()
	end

end 

function set_breaking_enabled(value)


end
