-- Util file that handles storing of fluids in a vessel, simulating the liquid and gaseous phases,
-- alongside thermodynamics (boiling of the fluids, condensation...) and allowing convenient extraction
-- of fluids.
-- To be used like a class
require("vehicle")
local plumbing = require("plumbing")

local fluid_container = {}
fluid_container.__index = fluid_container

-- m^3 Pa K^-1 mol^-1
local R = 8.314462618153


function fluid_container:new(volume)
    local container = {}
    setmetatable(container, fluid_container)

    -- Volume of the container in m^3
    container.volume = volume
    -- This extra volume can only have gases, useful to prevent infinite pressure
    container.extra_volume = 0.001
    -- From 0 to 1, 1 meaning the ullage (Gas) is uniformly distributed and not separated from the liquid
    container.ullage_distribution = 0
    -- Last acceleration experienced by the tank, useful to simulate column pressure of the fluid
    container.last_acceleration = 0
    -- Contents of the tank
    container.contents = plumbing.stored_fluids.new()

    return container
end

-- Pressure of the ullage portion (Pa)
function fluid_container:get_pressure()

end

function fluid_container:get_ullage_volume()
    return self.volume - self:get_fluid_volume()
end

function fluid_container:get_fluid_volume()
    local V = 0.0

    return V
end

-- Finds the point where VP = P via Newton's method
function fluid_container:go_to_equilibrium(max_dp)
    max_dp = max_dp or 0.1

end


-- Positive values mean inputting heat into the system, TODO: Units?
function fluid_container:exchange_heat(d)
    local cl_sum = 0.0
    local cg_sum = 0.0


end

-- This implements an extremely simple vapour-liquid equilibrium simulation
-- TODO: All of the contents are assumed not to react chemically
-- We also assume all substances evaporate at the same rate if they have the
-- same vapour pressure, we dont think about surface area as it would be overly complicated
function fluid_container:update(dt, acceleration)
    self.last_acceleration = math.max(acceleration, 0.0)

    local ullage_a_factor = 1.0
    local ullage_b_factor = 1.0
    local evp_factor = 1.0

    local d_ullage = -ullage_a_factor * (acceleration ^ 3.0) + ullage_b_factor
    self.ullage_distribution = self.ullage_distribution + d_ullage * dt
    self.ullage_distribution = math.max(self.ullage_distribution, 0.0)
    self.ullage_distribution = math.min(self.ullage_distribution, 1.0)



end




return fluid_container;