---@meta 

---@class fast_noise


local noise = {}

---@enum fast_noise.interp
noise.interp = {
  linear = 0,
  hermite = 1,
  quintic = 2
}

---@enum fast_noise.type
noise.fractal_type = {
  fbm = 0,
  billow = 1,
  rigid_multi = 2
}

---@enum fast_noise.cellular_distance
noise.cellular_distance = {
  euclidean = 0,
  manhattan = 1,
  natural = 2
}

---@enum fast_noise.cellular_return
noise.cellular_return = {
  cell_value = 0,
  noise_lookup = 1,
  distance = 2,
  distance2 = 3,
  distance2add = 4,
  distance2sub = 5
}

---@param fn fast_noise
---@param seed integer
function noise.set_seed(fn, seed) end

---@param fn fast_noise
---@param freq number
function noise.set_frequency(fn, freq) end

---@param fn fast_noise
---@param oct integer
function noise.set_fractal_octaves(fn, oct) end

---@param fn fast_noise
---@param gain number
function noise.set_fractal_gain(fn, gain) end

---@param fn fast_noise
---@param lac number
function noise.set_fractal_lacunarity(fn, lac) end

---@param fn fast_noise
---@param type fast_noise.type
function noise.set_fractal_type(fn, type) end

---@param fn fast_noise
---@param type fast_noise.interp
function noise.set_interp(fn, type) end

---@param fn fast_noise
---@param x number
---@param y number
---@nodiscard
---@return number
function noise.value2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@nodiscard
---@return number
function noise.value_fractal2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@nodiscard
---@return number
function noise.perlin2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@nodiscard
---@return number
function noise.perlin_fractal2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@nodiscard
---@return number
function noise.simplex2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@nodiscard
---@return number
function noise.simplex_fractal2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@nodiscard
---@return number
function noise.cellular2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@nodiscard
---@return number
function noise.cubic2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@nodiscard
---@return number
function noise.cubic_fractal2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@return number perturb_x
---@return number perturb_y
---@nodiscard
--- Slightly different behaviour, returns two values
function noise.gradient_perturb2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@return number perturb_x
---@return number perturb_y
---@nodiscard
--- Slightly different behaviour, returns two values
function noise.gradient_perturb_fractal2(fn, x, y) end

---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number
function noise.value3(fn, x, y, z) end

---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number
function noise.value_fractal3(fn, x, y, z) end

---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number
function noise.perlin3(fn, x, y, z) end

---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number
function noise.perlin_fractal3(fn, x, y, z) end

---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number
function noise.simplex3(fn, x, y, z) end

---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number
function noise.simplex_fractal3(fn, x, y, z) end

---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number
function noise.cellular3(fn, x, y, z) end


---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number
function noise.cubic3(fn, x, y, z) end

---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number
function noise.cubic_fractal3(fn, x, y, z) end

---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number x_perturb
---@return number y_perturb
---@return number z_perturb
--- Slightly different behaviour, returns 3 numbers
function noise.gradient_perturb3(fn, x, y, z) end

---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number x_perturb
---@return number y_perturb
---@return number z_perturb
--- Slightly different behaviour, returns 3 numbers
function noise.gradient_perturb_fractal3(fn, x, y, z) end


---@param fn fast_noise
---@param x number
---@param y number
---@param z number
---@param w number
---@nodiscard
---@return number
function noise.simplex4(fn, x, y, z, w) end


---@param fn fast_noise
---@param chance number
function noise.set_crater_chance(fn, chance) end

---@param fn fast_noise
---@param layers integer
function noise.set_crater_layers(fn, layers) end

---@param fn fast_noise
---@param do_rad boolean Whether to generate radial map
---@param x number
---@param y number
---@param z number
---@nodiscard
---@return number crater value
function noise.crater3(fn, do_rad, x, y, z) end

---@param fn fast_noise
---@return number radial component of last crater3 call
function noise.crater3_get_rad(fn) end

---@return fast_noise
---@param seed integer
function noise.new(seed) end


---@param seed integer
---@return fast_noise
function noise.new(seed) end

return noise
