require("game_database")


function load(database)

	database:add_part_category("categories/command.toml")
	database:add_part_category("categories/engines.toml")
	database:add_part_category("categories/all.toml")

	database:add_material("materials/hydrogen.toml")
	database:add_material("materials/oxygen.toml")
	database:add_material("materials/water.toml")

	-- The first locale is the default if selected one is not available
	database:load_locale(dofile("locale.lua"))


end


