-- This is similar to launchpad_finder but for whole space complexes
-- which contain many interactable buildings
-- bases must implement the get_base() function (and be an entity)

function add_base(universe, entity_id)
	universe.lua.bases[entity_id] = true
end

function remove_base(universe, entity_id)
	universe.lua.bases[entity_id] = nil
end

-- Warning: This returns a reference! Remember that in lua tables are passed by reference
function get_bases(universe)
	return universe.lua.bases
end

function init()
	universe.lua.bases = {}
end

