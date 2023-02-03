-- INCLUDE USING REQUIRE! THIS LIBRARY USES A GLOBAL OBJECT TO STORE THE LAUNCHPADS
-- Entities which have launchpads must provide a "get_launchpad(sub_id)" function which
-- returns the launchpad table

local launchpad_finder = {}

launchpad_finder.launchpads = {}

function launchpad_finder:add_launchpad(entity_uid)
	self.launchpads[entity_uid] = true
end

function launchpad_finder:remove_launchpad(entity_uid)
	self.launchpads[entity_uid] = nil
end

return launchpad_finder
