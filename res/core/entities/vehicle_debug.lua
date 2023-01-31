local imgui = require("imgui")

local vehicle_debug = {}

vehicle_debug.shown_machines = {}

function vehicle_debug:draw_main()
	self:parts_tab()
end

function vehicle_debug:draw_machines()
	for ruid, path in pairs(self.shown_machines) do 
		goto continue
		::hide_machine::
			self.shown_machines[ruid] = nil
		::continue::
			local part = self.vehicle:get_part_by_id(path["part"])
			if part == nil then goto hide_machine end
			local machine = part:get_machine(path["machine"])
			if machine == nil then goto hide_machine end
			if machine:draw_imgui() == false then goto hide_machine end
	end
end

function vehicle_debug:parts_tab()
	for _, part in ipairs(self.vehicle.parts) do
		imgui.push_id(part.id)
		local name = "Part (" .. part:get_part_proto():get_asset_id() .. ")" .. tostring(part.id)
		if imgui.collapsing_header(name) then
			imgui.text("Machines: ")
			for id, machine in pairs(part.machines) do 
				imgui.push_id(machine.runtime_uid)	
				imgui.text("\t" .. id .. "(" .. machine:get_id() .. ")")
				imgui.same_line()
				if self.shown_machines[machine.runtime_uid] ~= nil then 
					if imgui.button("Hide ImGui") then 
						self.shown_machines[machine.runtime_uid] = nil
					end
				else
					if imgui.button("Show ImGui") then
						local path = {}
						path["part"] = part.id
						path["machine"] = id
						self.shown_machines[machine.runtime_uid] = path
					end
				end
				imgui.pop_id()
			end
		end
		imgui.pop_id()
	end
end

---@param vehicle vehicle
function vehicle_debug:init(vehicle)
	self.vehicle = vehicle
	return self
end

return vehicle_debug
