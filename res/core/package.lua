require("game_database")
local logger = require("logger")
local glm = require("glm")

function load(database)


	database:add_part_category("categories/command.toml")
	database:add_part_category("categories/engines.toml")
	database:add_part_category("categories/all.toml")

  database:add_plumbing_machine("plumbing_machines/junction_3.toml")
  database:add_plumbing_machine("plumbing_machines/junction_4.toml")
  database:add_plumbing_machine("plumbing_machines/oneway.toml")
  database:add_plumbing_machine("plumbing_machines/shutoff.toml")
  database:add_plumbing_machine("plumbing_machines/electric_pump.toml")

	database:add_material("materials/hydrogen.toml")
	database:add_material("materials/oxygen.toml")
	database:add_material("materials/water.toml")

  database:add_reaction("materials/reactions/hydrogen_combustion.toml")
  database:add_reaction("materials/reactions/methane_combustion.toml")

	-- The first locale is the default if selected one is not available
	database:load_locale(dofile("locale.lua"))

end

