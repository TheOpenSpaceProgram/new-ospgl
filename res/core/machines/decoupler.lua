require("vehicle") 
require("bullet")
require("toml")
local glm = require("glm")
local debug_drawer = require("debug_drawer")

local force = machine.init_toml:get_number("force")
local d_point = machine.init_toml:get_string("decoupling_point")

-- TODO: Store this as a persistent thing, or deduce it from attachments
local decoupled = false


local function decouple()
	if not decoupled then
		print("Decoupling!")
		decoupled = true

		local separated = part:get_piece("p_root"):get_attached_to_marker(d_point)
		separated.attached_to = nil

		-- We update the vehicle to separate the pieces
		separated:set_dirty(true)

		force_dir = part:get_piece("p_root"):get_marker_forward(d_point)
		force_pos = part:get_piece("p_root"):get_marker_position(d_point)

		local rpos = separated:transform_point_to_rigidbody(force_pos)
		local rdir = separated:transform_axis(force_dir)
		separated.rigid_body:apply_impulse(rdir * force, rpos)
		part:get_piece("p_root").rigid_body:apply_impulse(-rdir * force, rpos)

		print("Done")
	end
end

local activable = machine:load_interface("core:interfaces/activable.lua")
activable.activate = decouple 

function update(dt)
end

