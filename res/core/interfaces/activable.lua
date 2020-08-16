local activable = create_interface()

activable.activated = false

function activable:activate()
	if not self.activated then
		self.activated = true
		if self.on_activate then
			self:on_activate()
		end
	end
end

function activable:deactivate()
	if self.deactivable and self.activated then
		self.activated = false
		if self.on_deactivate then
			self:on_deactivate()
		end
	end
end

-- (self) -> void
activable.on_activate = nil
-- (self) -> void
activable.on_deactivate = nil

activable.deactivable = true

return activable