-- Simulation of the combustion chamber
-------------------------------------------------------------------------
-- Simplistic, tunable model
--  -> Simple tunable evaporation model
--  -> Simulates TUNABLE reaction of the vaporized material
--  -> Tunable choked flow exhaust (potentially unrealistic, SIMILAR to real life equations)
-- May not conserve energy, but will conserve mass

local logger = require("logger")
local imgui = require("imgui")
local plumbing_lib = require("plumbing")
local toml = require("toml")

-- Chamber parameters
local chamber_volume = machine.init_toml:get_number("chamber_volume")
throat_area = machine.init_toml:get_number("throat_radius")^2 * math.pi
local chamber_temp = 288.0

local last_nozzle_gas_flow = 0.0
local last_nozzle_liquid_flow = 0.0
local last_heat_release = 0.0
local last_free_heat_release = 0.0
local last_temperature = 0.0
local last_pressure = 0.0
local last_mass = 0.0
local last_density = 0.0

-- m^3 Pa K^-1 mol^-1
local R = 8.314462618153

-- As fuel is nebulized, rate of evaporation is MASSIVE, not at all like on normal tanks
-- This is not a very realistic simulation! We could simulate it better approaching it like tanks do
-- Returns remaining heat
local function vaporize(tank, heat)
    local contents = tank:get_contents()
    -- We may consume up to this percent of heat to evaporate, to simulate
    -- "speed of evaporation", so that in low heat situations some fuel remains liquid
    local available_heat = heat * 0.9

    -- Calculate total heat required for full evaporation, including 
    -- heating up to the boiling point at current pressure 
    local required_heat = 0.0
    for phys_mat, stored_fluid in contents:pairs() do
        local evp = stored_fluid.liquid_mass
        required_heat = required_heat + evp * phys_mat.dH_vaporization
    end

    -- Dont mind if its a small ammount
    if required_heat <= 1000.0 then 
        return heat
    end

    if required_heat < available_heat then
        -- Total evaporation
        for phys_mat, stored_fluid in contents:pairs() do
            tank:set_vapor_fraction(phys_mat, 1.0)
        end

        return heat - required_heat
    else
        -- Consume all available heat proportionally
        for phys_mat, stored_fluid in contents:pairs() do
            local heat_for_this = stored_fluid.liquid_mass * phys_mat.dH_vaporization
            local evp = heat_for_this / required_heat
            tank:set_vapor_fraction(phys_mat, 1.0 - evp)
        end

        return heat - available_heat
    end
end

-- We drain proportionally, so that all gases have the same chance of going out
-- ALL REMAINING LIQUIDS ARE DRAINED EVERY FRAME!
local function drain_to_nozzle(tank, mass)    
    local expelled_fluid = plumbing_lib.stored_fluids.new()
    local contents = tank:get_contents()
    local total_gas_mass = tank:get_total_gas_mass()

    for phys_mat, stored_fluid in contents:pairs() do
        local proportion = mass * stored_fluid.gas_mass / total_gas_mass
        if proportion < 0 then 
            logger.info("mass= " .. mass .. ", stored_fluid.gas_mass = " .. stored_fluid.gas_mass .. 
                ", total_gas_mass = " .. total_gas_mass)
        end
        proportion = math.min(proportion, stored_fluid.gas_mass)
        tank:drain_to(expelled_fluid, phys_mat, stored_fluid.liquid_mass, proportion, true)
    end

    return expelled_fluid
end


-- Call from physics_update, returns fluids to be moved to the nozzle, pressure, and percent of choked flow satisfied
function simulate_chamber(dt)
    local tank = plumbing.internal_tank
    -- We carry the "combustion" on a iterated process, reacting a small quantity,
    -- evaporating, and repeating
    local iterations = 3 
    ldt = dt / iterations
    last_heat_release = 0.0
    last_free_heat_release = 0.0
    for i=1,iterations do
        local heat_released = tank:react(tank.temperature, chamber_volume, 0.001, ldt)
        local heat = heat_released
        last_heat_release = last_heat_release + heat_released
        heat = vaporize(tank, heat)
        tank:add_heat(heat)
        last_free_heat_release = last_free_heat_release + heat
    end

    last_heat_release = last_heat_release / dt 
    last_free_heat_release = last_free_heat_release / dt
    last_temperature = tank.temperature


    -- We assume liquids dont take space as a simplification
    -- We assume ideal gases PV = nRT => P = nRT / V
    local gR = tank:get_specific_gas_constant()
    local pressure = tank:get_total_gas_moles() * R * tank.temperature / chamber_volume
    last_pressure = pressure
    local density = tank:get_total_gas_mass() / chamber_volume
    last_density = density
    
    local gamma = 1.233
    -- TODO: Tunable "choked-flow-like" equation (for now expell everything)
    local choked_mass_flow = 99999999999999.0
    
    local drained = drain_to_nozzle(tank, choked_mass_flow * dt)

    last_nozzle_gas_flow = drained:get_total_gas_mass() / dt 
    last_nozzle_liquid_flow = drained:get_total_liquid_mass() / dt
    last_mass = tank:get_total_gas_mass() + tank:get_total_liquid_mass()


    return drained, pressure, 1.0
end

function get_last_chamber_pressure()
    return last_pressure
end

function draw_chamber_imgui()

    local str = "Mass flow: " .. tostring(last_nozzle_liquid_flow + last_nozzle_gas_flow) .. "kg/s"
    imgui.text(str)
    str = "Nozzle gas mass flow: " .. tostring(last_nozzle_gas_flow) .. "kg/s" 
    imgui.text(str)
    str = "Nozzle liquid mass flow: " .. tostring(last_nozzle_liquid_flow) .. "kg/s"
    imgui.text(str)
    imgui.text("\n")
    str = "Total heat released: " .. tostring(last_heat_release / 1000000) .. "MW"
    imgui.text(str)
    str = "Free heat released: " .. tostring(last_free_heat_release / 1000000) .. "MW"
    imgui.text(str)
    str = "Chamber temperature: " .. tostring(last_temperature - 273.15) .. "ºC"
    imgui.text(str)
    str = "Chamber pressure: " .. tostring(last_pressure / 101325) .. "atm"
    imgui.text(str)
    imgui.text("\n")
    str = "Chamber resident mass: " .. tostring(last_mass) .. "kg"
    imgui.text(str)
    str = "Chamber contents density: " .. tostring(last_density) .. "kg/m^3"
    imgui.text(str)

end