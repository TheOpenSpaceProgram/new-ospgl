---@meta 

---@class nano_vg.context
---@class nano_vg.paint
---@class nano_vg.color
---@class nano_vg.image

local nano_vg = {}

--- Use LuaJIT bit functions to or the horizontal with the vertical!
--- Example: bit.bor(nano_vg.text_align.left, nano_vg.text_align.middle)
---@enum nano_vg.text_align
nano_vg.text_align = {
  -- Horizontal align
  left = 0,
  center = 1,
  right = 4,
  -- Vertical align
  top = 8,
  middle = 16,
  bottom = 32,
  baseline = 64
}

---@param vg nano_vg.context
function nano_vg.save(vg) end
---@param vg nano_vg.context
function nano_vg.restore(vg) end
---@param vg nano_vg.context
function nano_vg.reset(vg) end
---@param vg nano_vg.context
---@param paint nano_vg.paint
function nano_vg.stroke_paint(vg, paint) end
---@param vg nano_vg.context
---@param paint nano_vg.paint
function nano_vg.fill_paint(vg, paint) end
---@param vg nano_vg.context
---@param width number
function nano_vg.stroke_width(vg, width) end
---@param vg nano_vg.context
function nano_vg.reset_transform(vg) end
---@param vg nano_vg.context
---@param a number
---@param b number
---@param c number
---@param d number
---@param e number
---@param f number
function nano_vg.transform(vg, a, b, c, d, e, f) end
---@param vg nano_vg.context
---@param x number
---@param y number
function nano_vg.translate(vg, x, y) end
---@param vg nano_vg.context
---@param angle number
function nano_vg.rotate(vg, angle) end
---@param vg nano_vg.context
---@param angle number
function nano_vg.skew_x(vg, angle) end
---@param vg nano_vg.context
---@param angle number
function nano_vg.skew_y(vg, angle) end
---@param vg nano_vg.context
---@param x number
---@param y number
function nano_vg.scale(vg, x, y) end

---@return nano_vg.paint
---@param vg nano_vg.context
---@param sx number
---@param sy number
---@param ex number
---@param ey number
---@param icol nano_vg.color
---@param ocol nano_vg.color
function nano_vg.linear_gradient(vg, sx, sy, ex, ey, icol, ocol) end
---@return nano_vg.paint
---@param vg nano_vg.context
---@param x number
---@param y number
---@param w number
---@param h number
---@param r number
---@param f number
---@param icol nano_vg.color
---@param ocol nano_vg.color
function nano_vg.box_gradient(vg, x, y, w, h, r, f, icol, ocol) end
---@return nano_vg.paint
---@param vg nano_vg.context
---@param cx number
---@param cy number
---@param inr number
---@param outr number
---@param icol nano_vg.color
---@param ocol nano_vg.color
function nano_vg.radial_gradient(vg, cx, cy, inr, outr, icol, ocol) end
---@return nano_vg.paint
---@param vg nano_vg.context
---@param ox number
---@param oy number
---@param ex number
---@param ey number
---@param angle number
---@param image nano_vg.image
---@param alpha number
function nano_vg.image_pattern(vg, ox, oy, ex, ey, angle, image, alpha) end

---@param vg nano_vg.context
---@param x number
---@param y number
---@param w number
---@param h number
function nano_vg.scissor(vg, x, y, w, h) end

---@param vg nano_vg.context
---@param x number
---@param y number
---@param w number
---@param h number
function nano_vg.intersect_scissor(vg, x, y, w, h) end

---@param vg nano_vg.context
function nano_vg.reset_scissor(vg) end

---@param vg nano_vg.context
function nano_vg.begin_path(vg) end

---@param vg nano_vg.context
---@param x number
---@param y number
function nano_vg.move_to(vg, x, y) end

---@param vg nano_vg.context
---@param x number
---@param y number
function nano_vg.line_to(vg, x, y) end

---@param vg nano_vg.context
---@param c1x number
---@param c1y number
---@param c2x number
---@param c2y number
---@param x number
---@param y number
function nano_vg.bezier_to(vg, c1x, c1y, c2x, c2y, x, y) end

---@param vg nano_vg.context
---@param cx number
---@param cy number
---@param x number
---@param y number
function nano_vg.quad_to(vg, cx, cy, x, y) end

---@param vg nano_vg.context
---@param x1 number
---@param y1 number
---@param x2 number
---@param y2 number
---@param radius number
function nano_vg.arc_to(vg, x1, y1, x2, y2, radius) end

---@param vg nano_vg.context
function nano_vg.close_path(vg) end

---@param vg nano_vg.context
---@param dir integer
function nano_vg.path_winding(vg, dir) end

---@param vg nano_vg.context
---@param cx number
---@param cy number
---@param r number
---@param a0 number
---@param a1 number
---@param dir integer
function nano_vg.arc(vg, cx, cy, r, a0, a1, dir) end

---@param vg nano_vg.context
---@param x number
---@param y number
---@param w number
---@param h number
function nano_vg.rect(vg, x, y, w, h) end

---@param vg nano_vg.context
---@param x number
---@param y number
---@param w number
---@param h number
---@param r number
function nano_vg.rounded_rect(vg, x, y, w, h, r) end

---@param vg nano_vg.context
---@param cx number
---@param cy number
---@param rx number
---@param ry number
function nano_vg.ellipse(vg, cx, cy, rx, ry) end

---@param vg nano_vg.context
---@param cx number
---@param cy number
---@param r number
function nano_vg.circle(vg, cx, cy, r) end

---@param vg nano_vg.context
function nano_vg.fill(vg) end

---@param vg nano_vg.context
function nano_vg.stroke(vg) end

---@param vg nano_vg.context
---@param size number
--- Doesn't work for bitmap fonts!
function nano_vg.font_size(vg, size) end

---@param vg nano_vg.context
---@param blur number
--- Doesn't work for bitmap fonts!
function nano_vg.font_blur(vg, blur) end

---@param vg nano_vg.context
---@param spacing number
function nano_vg.text_letter_spacing(vg, spacing) end

---@param vg nano_vg.context
---@param height number
function nano_vg.text_line_height(vg, height) end

---@param vg nano_vg.context
---@param align nano_vg.text_align
function nano_vg.text_align(vg, align) end

---@param vg nano_vg.context
---@param font string
function nano_vg.font_face(vg, font) end

---@param r integer
---@param g integer
---@param b integer
---@return nano_vg.color
function nano_vg.rgb(r, g, b) end

---@param r number
---@param g number
---@param b number
---@return nano_vg.color
function nano_vg.rgbf(r, g, b) end

---@param r integer
---@param g integer
---@param b integer
---@param a integer
---@return nano_vg.color
function nano_vg.rgba(r, g, b, a) end

---@param r number
---@param g number
---@param b number
---@param a number
---@return nano_vg.color
function nano_vg.rgbaf(r, g, b, a) end

---@param vg nano_vg.context
---@param color nano_vg.color
function nano_vg.fill_color(vg, color) end

---@param vg nano_vg.context
---@param color nano_vg.color
function nano_vg.stroke_color(vg, color) end

---@param a nano_vg.color
---@param b nano_vg.color
---@param u number
function nano_vg.lerp_rgba(a, b, u) end

---@param a nano_vg.color
---@param b nano_vg.color
---@param u integer
function nano_vg.trans_rgba(a, b, u) end

---@param a nano_vg.color
---@param b nano_vg.color
---@param u number
function nano_vg.trans_rgbaf(a, b, u) end

---@param h number
---@param s number
---@param l number
---@return nano_vg.color
function nano_vg.hsl(h, s, l) end

---@param h number
---@param s number
---@param l number
---@param a number
---@return nano_vg.color
function nano_vg.hsla(h, s, l, a) end


return nano_vg
