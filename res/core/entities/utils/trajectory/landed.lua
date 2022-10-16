-- A landed trajectory maintains fixed position relative to the surface of a body
local glm = require("glm")

local landed = {}

landed.universe = nil

-- ID of the body we are on  
landed.center_body = 0

-- Offset from the center of the body at t = 0
landed.initial_offset = glm.vec3.new(0, 0, 0)
landed.initial_rotation = glm.quat.new(1, 0, 0, 0)

-- Returns a table which contains pos, vel, rot and ang_vel
function landed:get_world_state(use_bullet)
    if use_bullet == nil then use_bullet = false end

    local out = {}
    local elem = universe.get_element(self.center_body)
    local tnow = 0.0
    if use_bullet then tnow = universe.system.bt else tnow = universe.system.t end
    
    local rot_matrix = elem.build_rotation_matrix(universe.system.t0, tnow, false)

    local state = nil
    if use_bullet then 
        state = universe.system.get_bullet_state(self.center_body)
    else
        state = universe.system.get_state(self.center_body)
    end

    local body_matrix = glm.translate(glm.mat4.new(1.0), state.pos)

    local rel_pos = glm.dvec3.new(rot_matrix * glm.dvec4.new(self.initial_offset, 1.0))
    local tform_pos = glm.dvec3.new(body_matrix * glm.dvec4.new(self.initial_offset, 1.0))
    local tform_rot = glm.dquat.new(rot_matrix * glm.toMat4(initial_rotation))

    out.pos = tform_pos
    out.rot = tform_rot
    
    local tangential = elem.get_tangential_speed(rel_pos)

    out.vel = state.vel + tangential
    -- TODO
    out.ang_vel = glm.vec3.new(0.0, 0.0, 0.0)

    return out
end

-- Sets center by body name
function landed:set_center(center_name)
    self.center_body = self.universe.get_element_index_from_name(center_name)
end

function landed:init(universe, center_name) 
    self.universe = universe
    self:set_center(center_name)
end

return landed