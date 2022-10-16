-- This file unifies the concept of launchpads, which are stored as a global array in the universe
-- launchpads must implement the get_launchpad() function
-- Launchpads may be added multiple times without problem

function add_launchpad(universe, entity_id)
    universe.lua.launchpads[entity_id] = true 
end

function remove_launchpad(universe, entity_id)
    universe.lua.launchpads[entity_id] = nil 
end

-- Warning: This returns a reference! Remember that in lua tables are passed by reference
function get_launchpads(universe)
    return universe.lua.launchpads
end

function init()
    universe.lua.launchpads = {}
end