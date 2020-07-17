require("vehicle") 
require("bullet")
require("toml")
local glm = require("glm")
local debug_drawer = require("debug_drawer")

local force = machine.init_toml:get_number("force")
local d_point = machine.init_toml:get_string("decoupling_point")

local function decouple()
	local root = part:get_piece("p_root")
	local attached = root:get_attached_to_marker(d_point)
	local separated = attached

	-- We cannot assume attached will be different from root as the link may be "inverted"
	if attached == root then separated = attached.attached_to end

	root:get_attached_to_marker(d_point).attached_to = nil

	-- We update the vehicle to separate the pieces so we can apply forces even if they 
	-- are welded
	separated:set_dirty(true)

	force_dir = root:get_marker_forward(d_point)
	force_pos = root:get_marker_position(d_point)

	local rpos = separated:transform_point_to_rigidbody(force_pos)
	local rdir = separated:transform_axis(force_dir)
	separated.rigid_body:apply_impulse(rdir * force, rpos)
	root.rigid_body:apply_impulse(-rdir * force, rpos)

end

local activable = machine:load_interface("core:interfaces/activable.lua")
activable.on_activate = decouple 
activable.deactivable = false

function update(dt)
end

