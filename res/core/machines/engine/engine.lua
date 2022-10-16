-- Based on Design of Liquid Propellant Rocket Engines, Huzel and Huang.
-- for reasonable approximation of real-life values

-- This file glues the chamber and nozzle simulators and gives everything to the engine interface

local logger = require("logger")
local imgui = require("imgui")
local toml = require("toml")

local nozzle_piece_str = machine.init_toml:get_string_or("nozzle_piece", "p_root")
local nozzle_piece = machine.in_part:get_piece(nozzle_piece_str)
local nozzle_marker = machine.init_toml:get_string("nozzle_marker")

local engine_interface = machine:load_interface("core:interfaces/engine.lua")

local last_thrust = 0.0
local last_pressure_thrust = 0.0
local last_choked = false
local last_exhaust_temp = 0.0
local last_exhaust_pressure = 0.0
local last_exhaust_mach = 0.0
local last_exhaust_area = 0.0

-- Include the simulators
dofile("machines/engine/chamber.lua")
dofile("machines/engine/nozzle.lua")

function physics_update(dt)
    engine_interface:physics_update(dt)
end


-- Implementation for the engine interface stuff

engine_interface.get_nozzle_position = function(self)
    local local_pos = nozzle_piece:get_marker_position(nozzle_marker)
    return nozzle_piece:transform_point_to_rigidbody(local_pos)
end

engine_interface.get_nozzle_forward = function(self)
    local local_fwd = -nozzle_piece:get_marker_forward(nozzle_marker)
    return nozzle_piece:transform_axis(local_fwd)
end

engine_interface.get_piece = function(self)
    return nozzle_piece
end

engine_interface.get_thrust = function(self, dt) 
    local to_nozzle, pressure, choke_factor = simulate_chamber(dt)
    local is_choked = choke_factor > 0.999

    --logger.info(to_nozzle:get_specific_gas_constant())
    local v, T, P, M = get_exit_properties(to_nozzle.temperature, pressure, to_nozzle:get_specific_gas_constant(), is_choked) 
    -- Thrust equation for a rocket engine
    -- TODO: When atmospheres are simulated, we must include the ambient pressure!
    local ambient_P = 0.0
    
    -- Effects of viscosity will ruin the pressure increase in the subsonic case due to turbulence
    -- so instead we simulate an effective exit pressure equal to that of the chamber, and an 
    -- effective exit_area of the throat area. This would physically mean a jet of material being
    -- expelled
    -- Furthermore, if the flow is very far away from being choked, the chamber pressure 
    -- will be closer and closer to ambient (TODO: We should really simulate this in the chamber itself)

    local effective_exit_area = exit_area
    local effective_P = P
    if not is_choked then
        effective_P = pressure * choke_factor + ambient_P * (1.0 - choke_factor)
        effective_exit_area = throat_area
    end

    last_exhaust_pressure = effective_P
    last_exhaust_mach = M
    last_exhaust_temp = T
    last_exhaust_velocity = v
    last_exhaust_area = effective_exit_area

    local thrust = v * to_nozzle:get_total_gas_mass() / dt
    -- Non vapor is seriously penalized in speed
    local non_vapor_thrust = v * 0.01 * to_nozzle:get_total_liquid_mass() / dt
    local pressure_thrust = (effective_P - ambient_P) * effective_exit_area

    last_thrust = thrust + non_vapor_thrust + pressure_thrust
    last_pressure_thrust = pressure_thrust
    return thrust
end 

function draw_imgui()
    draw_chamber_imgui()
    imgui.separator()
    if last_choked then
        imgui.text_colored(0.0, 1.0, 0.0, "Choked nozzle flow") 
    else
        imgui.text_colored(1.0, 0.0, 0.0, "Non-choked nozzle flow")
    end

    imgui.text("Effective exhaust area: " .. last_exhaust_area .. "m^2 (" .. 
        (last_exhaust_area / exit_area) * 100.0 .. " %)")
    imgui.text("Exhaust velocity: " .. last_exhaust_velocity .. "m/s (Mach = " .. last_exhaust_mach .. ")")
    imgui.text("Exhaust temperature: " .. last_exhaust_temp - 273.15 .. "ºC")
    imgui.text("Exhaust pressure: " .. last_exhaust_pressure / 101300 .. "atm")

    imgui.separator()
    imgui.text("Thrust: " .. last_thrust / 1000.0 .. "kN")
    imgui.text(" -> of which caused by pressure: " .. last_pressure_thrust / 1000.0 .. "kN (" .. 
        (last_pressure_thrust / last_thrust) * 100.0 .. " %)")
    
end

