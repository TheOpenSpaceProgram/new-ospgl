---@meta


local logger = {}

---@param message string
function logger.debug(message) end
---@param message string
function logger.info(message) end
---@param message string
function logger.warn(message) end
---@param message string
function logger.error(message) end
---@param message string
--- Causes a script crash so it won't continue (atleast for this frame!)
function logger.fatal(message) end

return logger
