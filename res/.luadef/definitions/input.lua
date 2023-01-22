---@meta 


---@enum input.key
--- Note: Don't worry about all indices being 0, they are properly set in the C++ API
local input = {
  key_unknown = 0,
  key_space = 0,
  key_apostrophe = 0,
  key_comma = 0,
  key_minus = 0,
  key_period = 0,
  key_slash = 0,
  key_0 = 0,
  key_1 = 0,
  key_2 = 0,
  key_3 = 0,
  key_4 = 0,
  key_5 = 0,
  key_6 = 0,
  key_7 = 0,
  key_8 = 0,
  key_9 = 0,
  key_semicolon = 0,
  key_equal = 0,
  key_a = 0,
  key_b = 0,
  key_c = 0,
  key_d = 0,
  key_e = 0,
  key_f = 0,
  key_g = 0,
  key_h = 0,
  key_i = 0,
  key_j = 0,
  key_k = 0,
  key_l = 0,
  key_m = 0,
  key_n = 0,
  key_o = 0,
  key_p = 0,
  key_q = 0,
  key_r = 0,
  key_s = 0,
  key_t = 0,
  key_u = 0,
  key_v = 0,
  key_x = 0,
  key_y = 0,
  key_z = 0,
  key_left_bracket = 0,
  key_backslash = 0,
  key_right_bracket = 0,
  key_grave_accent = 0,
  key_world_1 = 0,
  key_world_2 = 0,
  key_escape = 0,
  key_enter = 0,
  key_tab = 0,
  key_backspace = 0,
  key_insert = 0,
  key_delete = 0,
  key_right = 0,
  key_left = 0,
  key_down = 0,
  key_up = 0,
  key_page_up = 0,
  key_page_down = 0,
  key_home = 0,
  key_end = 0,
  key_caps_lock = 0,
  key_scroll_lock = 0,
  key_num_lock = 0,
  key_print_screen = 0,
  key_pause = 0,
  key_f1 = 0,
  key_f2 = 0,
  key_f3 = 0,
  key_f4 = 0,
  key_f5 = 0,
  key_f6 = 0,
  key_f7 = 0,
  key_f8 = 0,
  key_f9 = 0,
  key_f10 = 0,
  key_f11 = 0,
  key_f12 = 0,
  key_f13 = 0,
  key_f14 = 0,
  key_f15 = 0,
  key_f16 = 0,
  key_f17 = 0,
  key_f18 = 0,
  key_f19 = 0,
  key_f20 = 0,
  key_f21 = 0,
  key_f22 = 0,
  key_f23 = 0,
  key_f24 = 0,
  key_f25 = 0,
  key_kp_0 = 0,
  key_kp_1 = 0,
  key_kp_2 = 0,
  key_kp_3 = 0,
  key_kp_4 = 0,
  key_kp_5 = 0,
  key_kp_6 = 0,
  key_kp_7 = 0,
  key_kp_8 = 0,
  key_kp_9 = 0,
  key_kp_decimal = 0,
  key_kp_divide = 0,
  key_kp_multiply = 0,
  key_kp_substract = 0,
  key_kp_add = 0,
  key_kp_enter = 0,
  key_kp_equal = 0,
  key_left_shift = 0,
  key_left_control = 0,
  key_left_alt = 0,
  key_left_super = 0,
  key_right_shift = 0,
  key_right_control = 0,
  key_right_alt = 0,
  key_right_super = 0,
  key_menu = 0
}


---@param key input.key
---@return boolean
function input:key_pressed(key) end

---@param key input.key
---@return boolean
function input:key_down(key) end

---@param key input.key
---@return boolean
function input:key_down_or_repeating(key) end

---@param key input.key
---@return boolean
function input:key_up(key) end

---@param btn input.mouse
---@return boolean
function input:mouse_pressed(btn) end

---@param btn input.mouse
---@return boolean
function input:mouse_down(btn) end

---@param btn input.mouse
---@return boolean
function input:mouse_up(btn) end

---@param cursor input.cursor
function input:set_cursor(cursor) end

---@return string
function input:get_input_text() end

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
