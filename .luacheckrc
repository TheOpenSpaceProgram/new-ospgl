-- Some stuff which doesn't matter and is just annoying
-- As a lot of functions are externally called, we also ignore "unused function [x]"

ignore = {"212", "611", "612", "613", "614"}

always_globals = {"__pkg"}
always_read_globals = {"require"}

-- We assume a path convention for lua files, feel free to ignore it though,
-- but you won't get automatic luacheck configuration

local function setup(path, g, rg)

	local nglobals = always_globals
	local nread_globals = always_read_globals

	for k, v in ipairs(g) do table.insert(nglobals, v) end 
	for k, v in ipairs(rg) do table.insert(nread_globals, v) end	

	files[path].globals = nglobals
	files[path].read_globals = nread_globals

	for k, v in ipairs(nglobals) do
 		print("[" .. k .. "] = " .. tostring(v))
	end 

end

setup("**/**/machines/**/**.lua", 
{	"machine", "init", "part", "piece", "define_ports",
	"update", "pre_update", "get_input_context" },
{})

setup("**/**/planets/**/**.lua",
{"generate"},
{})

setup("**/**/links/**/**.lua",
{"load_toml", "activate", "deactivate", "is_broken", "set_breaking_enabled"},
{})
