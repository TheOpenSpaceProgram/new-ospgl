require("game_database")

function load(database)

	database:add_part_category("categories/command.toml")
	database:add_part_category("categories/engines.toml")
	database:add_part_category("categories/all.toml")

	database:add_material("materials/hydrogen.toml")
	database:add_material("materials/oxygen.toml")
	database:add_material("materials/water.toml")

end


