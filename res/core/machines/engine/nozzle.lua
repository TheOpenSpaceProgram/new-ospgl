local logger = require("logger")
local imgui = require("imgui")
local plumbing_lib = require("plumbing")
local toml = require("toml")

-- Returns two values, supersonic exit mach, and subsonic exit mach
-- The subsonic case assumes that fluid almost reaches mach 1 in the throat,
-- which may not be realistic in many cases
-- Source: https://www.grc.nasa.gov/WWW/winddocs/utilities/b4wind_guide/mach.html
-- gamma is the specific heat ratio as usual
-- Only needs to be called when the exit area / gamma changes
local function compute_exit_mach(throat_area, exit_area, gamma)
    local Ar = throat_area / exit_area
    local C = ((gamma + 1) / (gamma - 1)) / 2
    local ct1 = (gamma + 1) / 2 
    local ct2 = (gamma - 1) / 2
    -- We must solve Ar = M (ct1 / (1 + ct2 * M ^ 2))^C for M
    -- so an iterative method is implemented
    -- Note that we are solving for the exit value of M!
    
    -- Initial guess (sup = supersonic, sub = subsonic)
    local P = 2 / (gamma + 1)
    local Q = 1 - P
    local E = 1 / Q 
    local Rsub = (1 / Ar) ^ 2 
    local Rsup = (1 / Ar) ^ (2 * Q / P)
    local asub = P ^ (1 / Q)
    local asup = Q ^ (1 / P)
    local rsub = (Rsub - 1) / (2 * asub)
    local rsup = (Rsup - 1) / (2 * asup)
    local xsub = 1 / (1 + rsub + math.sqrt(rsub * (rsub + 2)))
    local xsup = 1 / (1 + rsup + math.sqrt(rsup * (rsup + 2)))

    -- Iterative method
    -- We use the observation in the source that supersonic and subsonic
    -- cases are very similar, and just exchange P and Q for each
    local function iterate(P, Q, R, X)
        return P * (X - 1) / (1 - R * (P + Q * X) ^ (-P / Q))
    end
    
    for i=1,2 do 
        xsub = iterate(P, Q, rsub, xsub)
        xsup = iterate(Q, P, rsup, xsup)
    end
    
    local Msub = math.sqrt(xsub)
    local Msup = 1 / math.sqrt(xsup)

    return Msub, Msup

end

-- TODO: Allow variable geometry nozzles, should be relatively easy
-- TODO: Allow varying gamma? Probably overkill
-- but we could compute the relations for a variety of gammas 
-- ie, for monoatomic, diatomic and triatomic gases, and then interpolate between
-- these, to obtain a really good approximation, as monoatomic gases may be used
-- in some cold gas thrusters, and triatomic gases are common in exhaust!
local gamma = 1.233

exit_area = machine.init_toml:get_number("exit_radius")^2 * math.pi
local throat_area = machine.init_toml:get_number("throat_radius")^2 * math.pi

local exit_M_subsonic, exit_M_supersonic = compute_exit_mach(throat_area, exit_area, gamma)

local last_exhaust_velocity = 0.0
local last_exhaust_mach = 0.0
local last_choked = false
local last_exhaust_temp = 0.0
local last_exhaust_pressure = 0.0

-- Ideal, so we can use a = sqrt(gamma * R * T / Mm)
-- TODO: Once again, we should think about allowing varying gamma
-- Returns Mach, Temperature and Pressure
function get_exit_properties(temperature, pressure, specific_gas_ct, is_choked)
    last_choked = is_choked

    local M = exit_M_subsonic
    
    if is_choked then 
        M = exit_M_supersonic
    end

    last_exhaust_mach = M

    local v = M * math.sqrt(gamma * temperature * specific_gas_ct)

    last_exhaust_velocity = v

    -- Isentropic flow, so we can use isentropic relationships
    -- We assume that at the inlet M = 1 
    -- TODO: This may not be really correct for non-choked flow
    local ct = (gamma - 1) / 2
    local ct2 = gamma / (gamma - 1)
    
    local Tt1 = temperature * (1.0 + ct)
    local T = Tt1 / (1.0 + ct * M^2) 
    local Pt1 = pressure * (1.0 + ct) ^ ct2 
    local P = Pt1 / ((1.0 + ct * M^2)^ct2)

    -- TODO: This is an adjustment for non-choked flow, but it's
    -- not realistic. Or is it?
    if P > pressure then P = pressure end

    last_exhaust_temp = T 
    last_exhaust_pressure = P

    return v, T, P
end

function draw_nozzle_imgui()
    if last_choked then
        imgui.text_colored(0.0, 1.0, 0.0, "Choked nozzle flow") 
    else
        imgui.text_colored(1.0, 0.0, 0.0, "Non-choked nozzle flow")
    end

    imgui.text("Exhaust velocity: " .. last_exhaust_velocity .. "m/s (Mach = " .. last_exhaust_mach .. ")")
    imgui.text("Exhaust temperature: " .. last_exhaust_temp - 273.15 .. "ºC")
    imgui.text("Exhaust pressure: " .. last_exhaust_pressure / 101300 .. "atm")
end
