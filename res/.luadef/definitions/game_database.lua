---@meta 

---@class game_database
--- The game_database, at the current point, is used to specify
--- assets that will be used by the game engine. As the editor is 
--- implemented in C++ (and thus "belongs" to the engine), editor stuff 
--- is defined from the game_database.
--- For lua stuff, you can use global tables as a sort of database.
--- Remember that state is shared across all scripts (except planet generators)
--- so use appropiate naming for your global tables.
local game_database = {}

---@param path string Path to the part .toml
function game_database:add_part(path) end

---@param path string Path to the category .toml
function game_database:add_part_category(path) end

---@param path string Path to the physical material .toml
function game_database:add_material(path) end

---@param path string Path to the plumbing machine .toml
function game_database:add_plumbing_machine(path) end

---@param path string Path to the reaction .toml
function game_database:add_reaction(path) end

---@param locale table Table containing the locale
function game_database:load_locale(locale) end

---@param id string Identifier of the localized string 
---@return string String to display
function game_database:get_string(id) end

---@param path string Path to the editor script
--- Note that editor scripts should NOT interact with the universe or similar!
--- Furthermore, make sure your event handler is global to prevent it from being 
--- garbage collected, as the editor will just load the scripts once, and then all locals
--- are up for gargabe collection (unless of course used by event handlers)
function game_database:add_editor_script(path) end

-- We dont' return anything, simply define the game_database class!
