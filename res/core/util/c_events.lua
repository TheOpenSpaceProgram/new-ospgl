-- A more comfortable method to handle events, extremely simple
require("events")
local events = {}

events.handlers = {}

---@param event string Name of the event
---@param id string Name to give to the id for later removal
---@param func function
function events:add_named(emitter, event, id, func)
	self.handlers[id] = emitter:sign_up_for_event(event, func)
end

---@param event string Name of the event
---@param func function
--- Does not allow later disabling of the event, only by destruction of the whole event handler
--- or removal of ALL event handlers
function events:add(emitter, event, func)
	table.insert(self.handlers, emitter:sign_up_for_event(event, func))
end

function events:remove_all()
	for _, handler in pairs(self.handlers) do
		handler:sign_out()
	end
	self.handlers = {}
end

---@param id string Name of the event id to remove
function events:remove(id)
	if self.handlers[id] then
		self.handlers[id]:sign_out()
	end
	self.handlers[id] = nil
end

return events
