-- INCLUDE USING REQUIRE! THIS LIBRARY USES A GLOBAL OBJECT TO STORE THE LAUNCHPADS
-- Entities which have launchpads must provide a "get_launchpad(sub_id)" function which
-- returns the launchpad table

local launchpad_finder = {}

---@type table<integer, boolean>
launchpad_finder.launchpads = {}

function launchpad_finder:add_launchpad(entity_uid)
	self.launchpads[entity_uid] = true
end

function launchpad_finder:remove_launchpad(entity_uid)
	self.launchpads[entity_uid] = nil
end

---@return table<string, core.launchpad>
--- Returns an empty table if no launchpads could be found at said entity
function launchpad_finder:get_launchpads(entity_uid)
	local ent = osp.universe:get_entity(entity_uid)
	if ent and ent.lua.get_launchpads then
		return ent.lua.get_launchpads()
	end
	return {}
end

return launchpad_finder
