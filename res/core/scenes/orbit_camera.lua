local input = require("input")
local glm = require("glm")
local cameras = require("core:scenes/camera_util.lua")

local orbit_camera = {}

orbit_camera.azimuth = 0.0
orbit_camera.altitude = math.pi / 2
orbit_camera.x_speed = 0.0
orbit_camera.y_speed = 0.0
orbit_camera.zoom_speed = 0.0
orbit_camera.radius = 100.0
orbit_camera.fov = 40.0

orbit_camera.sensitivity = 0.01
orbit_camera.inertia_damp = 4.0
orbit_camera.zoom_damp = 12.0

---@type fun(cam): number
orbit_camera.get_min_radius = nil
---@type fun(cam): glm.vec3
orbit_camera.get_center = nil
---@type fun(cam): glm.vec3
orbit_camera.get_pole = nil
---@type gui.input
orbit_camera.gui_input = nil

function orbit_camera:get_camera_uniforms(width, height)
	return cameras.from_center_and_polar(self:get_center(), self:get_pole(),
		self.azimuth, self.altitude, self.radius, self.fov, width, height)
end

function orbit_camera:update(dt)
	if not self.gui_input.ext_mouse_blocked and not self.gui_input.mouse_blocked then
		self.zoom_speed = self.zoom_speed + input.get_scroll()
		self.radius = self.radius * (1.0 - self.zoom_speed * dt * 0.5)
		self.radius = math.max(self.radius, self:get_min_radius())

		if self.zoom_speed > 0 then
			self.zoom_speed = self.zoom_speed - self.zoom_damp * dt
			if self.zoom_speed < 0 then self.zoom_speed = 0 end
		elseif self.zoom_speed < 0 then
			self.zoom_speed = self.zoom_speed + self.zoom_damp * dt
			if self.zoom_speed > 0 then self.zoom_speed = 0 end
		end

		if input.mouse_pressed(input.btn.right) then
			self.azimuth = self.azimuth + input.get_mouse_delta().x * self.sensitivity
			self.altitude = self.altitude - input.get_mouse_delta().y * self.sensitivity
			self.x_speed = input.get_mouse_delta().x * self.sensitivity * 10.0
			self.y_speed = -input.get_mouse_delta().y * self.sensitivity * 10.0
		else
			if self.x_speed > 0 then
				self.x_speed = self.x_speed - self.inertia_damp * dt
				if self.x_speed < 0 then self.x_speed = 0 end
			elseif self.x_speed < 0 then
				self.x_speed = self.x_speed + self.inertia_damp * dt
				if self.x_speed > 0 then self.x_speed = 0 end
			end
			if self.y_speed > 0 then
				self.y_speed = self.y_speed - self.inertia_damp * dt
				if self.y_speed < 0 then self.y_speed = 0 end
			elseif self.y_speed < 0 then
				self.y_speed = self.y_speed + self.inertia_damp * dt
				if self.y_speed > 0 then self.y_speed = 0 end
			end

			self.azimuth = self.azimuth + self.x_speed * dt
			self.altitude = self.altitude + self.y_speed * dt
		end

		self.altitude = glm.clamp(self.altitude, 0.01, math.pi - 0.01)
	end
end

return orbit_camera
