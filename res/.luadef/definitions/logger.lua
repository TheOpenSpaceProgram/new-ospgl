---@meta


local logger = {}

---@param message any Must be castable to string
function logger.debug(message) end
---@param message any Must be castable to string
function logger.info(message) end
---@param message any Must be castable to string
function logger.warn(message) end
---@param message any Must be castable to string
function logger.error(message) end
---@param message any Must be castable to string
--- Causes a script crash so it won't continue (atleast for this frame!)
function logger.fatal(message) end

return logger
