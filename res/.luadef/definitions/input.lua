---@meta 


---@class input
local input = {}

---@return glm.vec2
function input.get_mouse_delta() end

---@return glm.vec2
function input.get_mouse_pos() end

---@return glm.vec2
function input.get_prev_mouse_pos() end

---@return number
function input.get_scroll() end

---@return number
function input.get_scroll_delta() end

---@return number
function input.get_prev_scroll() end

---@return number
function input.get_scroll() end

---@enum input.key
--- Note: Don't worry about all indices being 0, they are properly set in the C++ API
input.key = {
  unknown = 0,
  space = 0,
  apostrophe = 0,
  comma = 0,
  minus = 0,
  period = 0,
  slash = 0,
  k0 = 0,
  k1 = 0,
  k2 = 0,
  k3 = 0,
  k4 = 0,
  k5 = 0,
  k6 = 0,
  k7 = 0,
  k8 = 0,
  k9 = 0,
  semicolon = 0,
  equal = 0,
  a = 0,
  b = 0,
  c = 0,
  d = 0,
  e = 0,
  f = 0,
  g = 0,
  h = 0,
  i = 0,
  j = 0,
  k = 0,
  l = 0,
  m = 0,
  n = 0,
  o = 0,
  p = 0,
  q = 0,
  r = 0,
  s = 0,
  t = 0,
  u = 0,
  v = 0,
  x = 0,
  y = 0,
  z = 0,
  left_bracket = 0,
  backslash = 0,
  right_bracket = 0,
  grave_accent = 0,
  world_1 = 0,
  world_2 = 0,
  escape = 0,
  enter = 0,
  tab = 0,
  backspace = 0,
  insert = 0,
  delete = 0,
  right = 0,
  left = 0,
  down = 0,
  up = 0,
  page_up = 0,
  page_down = 0,
  home = 0,
  kend = 0,
  caps_lock = 0,
  scroll_lock = 0,
  num_lock = 0,
  print_screen = 0,
  pause = 0,
  f1 = 0,
  f2 = 0,
  f3 = 0,
  f4 = 0,
  f5 = 0,
  f6 = 0,
  f7 = 0,
  f8 = 0,
  f9 = 0,
  f10 = 0,
  f11 = 0,
  f12 = 0,
  f13 = 0,
  f14 = 0,
  f15 = 0,
  f16 = 0,
  f17 = 0,
  f18 = 0,
  f19 = 0,
  f20 = 0,
  f21 = 0,
  f22 = 0,
  f23 = 0,
  f24 = 0,
  f25 = 0,
  kp_0 = 0,
  kp_1 = 0,
  kp_2 = 0,
  kp_3 = 0,
  kp_4 = 0,
  kp_5 = 0,
  kp_6 = 0,
  kp_7 = 0,
  kp_8 = 0,
  kp_9 = 0,
  kp_decimal = 0,
  kp_divide = 0,
  kp_multiply = 0,
  kp_substract = 0,
  kp_add = 0,
  kp_enter = 0,
  kp_equal = 0,
  left_shift = 0,
  left_control = 0,
  left_alt = 0,
  left_super = 0,
  right_shift = 0,
  right_control = 0,
  right_alt = 0,
  right_super = 0,
  menu = 0
}


---@param key input.key
---@return boolean
function input.key_pressed(key) end

---@param key input.key
---@return boolean
function input.key_down(key) end

---@param key input.key
---@return boolean
function input.key_down_or_repeating(key) end

---@param key input.key
---@return boolean
function input.key_up(key) end

---@param btn input.mouse
---@return boolean
function input.mouse_pressed(btn) end

---@param btn input.mouse
---@return boolean
function input.mouse_down(btn) end

---@param btn input.mouse
---@return boolean
function input.mouse_up(btn) end

---@param cursor input.cursor
function input.set_cursor(cursor) end

---@return string
function input.get_input_text() end

---@enum input.cursor
input.cursor = {
  normal = 0,
  ibeam = 1,
  crosshair = 2,
  hand = 3,
  resize_ew = 4,
  resize_ns = 5,
  resize_nwse = 6,
  resize_nesw = 6,
  resize_all = 7,
  not_allowed = 8
}

---@enum input.mouse
input.btn = {
  left = 0,
  right = 1,
  middle = 2
}


return input
