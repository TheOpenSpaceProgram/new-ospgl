-- Util file that handles storing of fluids in a vessel, simulating the liquid and gaseous phases,
-- alongside thermodynamics (boiling of the fluids, condensation...) and allowing convenient extraction
-- of fluids. It assumes the fluids wont mix or react together, which may be too simple for some use cases!
-- To be used like a class
require("vehicle")
local logger = require("logger")
local plumbing = require("plumbing")
local assets = require("assets")
local noise = require("noise")

local fluid_container = {}
fluid_container.__index = fluid_container

-- m^3 Pa K^-1 mol^-1
local R = 8.314462618153

local function get_partial_pressure(temp, volume, phys_mat, stored_fluid)
    return phys_mat:get_moles(stored_fluid.gas_mass) * R * temp / volume;
end

-- Volume in m^3!
function fluid_container.new(volume)
    assert(type(volume) == "number")

    local container = {}
    setmetatable(container, fluid_container)

    -- Noise generator for gas flow TODO: Set the seed to some other value?
    container.noise_gen = noise.new(0)
    container.noise_t = 0.0
    -- Volume of the container in m^3
    container.volume = volume
    -- This extra volume can only have gases, useful to prevent infinite pressure
    container.extra_volume = 0.001
    -- From 0 to 1, 1 meaning the ullage (Gas) is uniformly distributed and not separated from the liquid
    container.ullage_distribution = 0
    -- Last acceleration experienced by the tank, useful to simulate column pressure of the fluid
    container.last_acceleration = 0
    -- Temperature of the contents of the tank, assumed to be homogeneous
    container.temperature = 373
    -- Contents of the tank
    container.contents = plumbing.stored_fluids.new()
    local h2 = assets.get_physical_material("materials/hydrogen.toml")
    local o2 = assets.get_physical_material("materials/oxygen.toml")
    container.contents:add_fluid(h2, 5.0, 0.0)
    container.contents:add_fluid(o2, 5.0, 0.0)

    return container
end

-- Pressure of the ullage portion (Pa)
function fluid_container:get_pressure()
    local p = 0.0
    local total_ullage = self:get_ullage_volume() + self.extra_volume
    local contents = self.contents:get_contents()
    for phys_mat, stored_fluid in contents:pairs() do
        p = p + get_partial_pressure(self.temperature, total_ullage, phys_mat, stored_fluid)
    end
    return p
end

-- Obtains pressure due to the weight of the fluid, depends on acceleration last frame (Pa)
-- Note that depth will depend on the geometry of the tank
-- Negative accelerations will result in negative pressure!
function fluid_container:get_column_pressure(depth)
    local p = 0.0

    -- We accumulate the pressure exerted by each liquid, assuming no mixing
    local contents = self.contents:get_contents()
    for phys_mat, stored_fluid in contents:pairs() do
        -- We consider the thresold the ammount required to fill 1% of the tank
        -- This prevents "droplets" from exerting full pressure on the tank
        local thresold = self.volume * phys_mat.liquid_density * 0.01
        local factor = stored_fluid.liquid_mass / thresold
        factor = math.min(factor, 1.0)
        p = p + phys_mat.liquid_density * depth * self.last_acceleration * factor
    end

    return p

end

function fluid_container:get_ullage_volume()
    return self.volume - self:get_fluid_volume()
end

function fluid_container:get_fluid_volume()
    local v = 0.0
    local contents = self.contents:get_contents()
    for phys_mat, stored_fluid in contents:pairs() do
        v = v + stored_fluid.liquid_mass / phys_mat.liquid_density
    end
    return v
end

-- Finds the point where VP = P via Newton's method
function fluid_container:go_to_equilibrium(max_dp)
    max_dp = max_dp or 0.1
    local ullage_0 = self.ullage_distribution
    local P0 = self:get_pressure()
    local T0 = self.temperature
    local softening = 1.0
    local it = 0

    while true do
        self:update(5.0 * softening, 0.0)
        self.temperature = T0
        local P = self:get_pressure()
        local dP = P - P0
        if math.abs(dP) < max_dp then
            break
        end

        if it >= 50 then
            logger.warn("Too many iterations trying to solve tank. Early exit!")
            break
        end

        P0 = P
        it = it + 1
        softening = softening * 0.96

    end

    self.ullage_distribution = ullage_0

end

-- Positive values mean inputting heat into the system (In J)
function fluid_container:exchange_heat(d)
    local cl_sum = 0.0
    local cg_sum = 0.0

    local contents = self.contents:get_contents()
    for phys_mat, stored_fluid in contents:pairs() do
        local cl = phys_mat.heat_capacity_liquid
        local cg = phys_mat.heat_capacity_gas
        cl_sum = cl_sum + cl * stored_fluid.liquid_mass
        cg_sum = cg_sum + cg * stored_fluid.gas_mass
    end

    local c_sum = cl_sum + cg_sum
    self.temperature = self.temperature + d / c_sum

end

-- This implements an extremely simple vapour-liquid equilibrium simulation
-- TODO: All of the contents are assumed not to react chemically
-- We also assume all substances evaporate at the same rate if they have the
-- same vapour pressure, we dont think about surface area as it would be overly complicated
function fluid_container:update(dt, acceleration)
    self.noise_t = self.noise_t + dt
    self.last_acceleration = math.max(acceleration, 0.0)

    local ullage_a_factor = 1.0
    local ullage_b_factor = 1.0
    local evp_factor = 1.0

    local d_ullage = -ullage_a_factor * (acceleration ^ 3.0) + ullage_b_factor
    self.ullage_distribution = self.ullage_distribution + d_ullage * dt
    self.ullage_distribution = math.max(self.ullage_distribution, 0.0)
    self.ullage_distribution = math.min(self.ullage_distribution, 1.0)

    local total_ullage = self:get_ullage_volume() + self.extra_volume;

    local contents = self.contents:get_contents()
    for phys_mat, stored_fluid in contents:pairs() do
        local cp = get_partial_pressure(self.temperature, total_ullage, phys_mat, stored_fluid)
        local vp = phys_mat:get_vapor_pressure(self.temperature)
        local dp = vp - cp;
        local moles_needed = (dp * total_ullage) / (R * self.temperature)
        local dm = phys_mat:get_mass(moles_needed) * evp_factor * dt
        if dm > 0 then
            dm = math.min(stored_fluid.liquid_mass, dm)
        else
            local allowed_mass = self:get_ullage_volume() * phys_mat.liquid_density;
            dm = -math.min(stored_fluid.gas_mass, -dm)
            dm = -math.min(allowed_mass, -dm)

        end

        -- We exchange energy for the thermodynamic process, if the energy is not
        -- available, it won't happen (TODO?)
        self:exchange_heat(-phys_mat.dH_vaporization * dm)

        stored_fluid.liquid_mass = stored_fluid.liquid_mass - dm;
        stored_fluid.gas_mass = stored_fluid.gas_mass + dm;
    end
end

function fluid_container:get_total_pressure(depth)
    depth = depth or 0.0
    return self:get_pressure() + self:get_column_pressure(depth)
end

-- Attempts to get flow volume (m^3) of liquid out of the tank
-- If ullage is not good, gas may also be drained!
-- It may fail and return less than asked for if there's not enough materials
-- Drains uniformly from all liquid layers (TODO: Maybe drain using density as if there were layers?)
function fluid_container:get_liquid(flow, do_flow)
    local contents = self.contents:get_contents()
    local total_liquid_vol = 0.0
    local result = plumbing.stored_fluids.new()

    -- We do a weighted average for liquids, gases are drained relative to ullage volume
    for phys_mat, stored_fluid in contents:pairs() do
        total_liquid_vol = total_liquid_vol + stored_fluid.liquid_mass / phys_mat.liquid_density
    end


    if total_liquid_vol == 0.0 then
        -- We can only drain gases now
        for phys_mat, stored_fluid in contents:pairs() do
            -- We adjust the flow to current gas density
            local flow_mass = flow * stored_fluid.gas_mass / self:get_ullage_volume()
            self.contents:drain_to(result, phys_mat, 0.0, flow_mass, do_flow)
        end

    else
        for phys_mat, stored_fluid in contents:pairs() do
            local liquid_mass = flow * stored_fluid.liquid_mass / total_liquid_vol
            -- We drain gases on a pseudorandom phasion using the noise generator
            local gas_mass = noise.perlin2(self.noise_gen, self.noise_t * 50.0, 0.0) * self.ullage_distribution
            self.contents:drain_to(result, phys_mat, liquid_mass, gas_mass, do_flow)
        end
    end

    return result

end

function fluid_container:add_liquid(flow, do_flow)

end

-- TODO: implement imgui functions in LUA
function fluid_container:draw_imgui()

end

return fluid_container;