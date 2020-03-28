local bullet = require("bullet");
local glm = require("glm");

local constraint = nil;
local bt_world = nil;

function load_toml(toml_data)

end

function activate(from_rigid, from_frame, to_rigid, to_frame, world)
	if constraint == nil then 
		bt_world = world;
		constraint = bullet.generic_6dof_constraint.new(from_rigid, to_rigid, from_frame, to_frame, true);
		constraint:add_to_world(world, true);
	end
end

function deactivate()	
	if constraint ~= nil then 
		constraint:remove_from_world(bt_world);
		bt_world = nil;
		constraint = nil;
	end

end 

function is_broken() 

	return not constraint:is_enabled();

end 

function set_breaking_enabled(value)


end
