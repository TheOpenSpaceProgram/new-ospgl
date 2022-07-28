#include "LuaNanoVG.h"
#include <nanovg/nanovg.h>

void LuaNanoVG::load_to(sol::table &table)
{
	sol::state_view sview(table.lua_state());
	sview.open_libraries(sol::lib::ffi);

	// Load all the FFI stuff
	auto res = sview.script("ffi.cdef[["
  		"void nvgSave(struct NVGcontext* ctx);"
  		"void nvgRestore(struct NVGcontext* ctx);"
		"void nvgReset(struct NVGcontext* ctx);"
		"void nvgStrokePaint(struct NVGcontext* ctx, struct NVGpaint paint);"
		"void nvgFillPaint(struct NVGcontext* ctx, struct NVGpaint paint);"
  		"void nvgStrokeWidth(struct NVGcontext* ctx, float size);"
		"void nvgResetTransform(struct NVGcontext* ctx);"
  		"void nvgTransform(struct NVGcontext* ctx, float a, float b, float c, float d, float e, float f);"
		"void nvgTranslate(struct NVGcontext* ctx, float x, float y);"
		"void nvgRotate(struct NVGcontext* ctx, float angle);"
		"void nvgSkewX(struct NVGcontext* ctx, float angle);"
		"void nvgSkewY(struct NVGcontext* ctx, float angle);"
		"void nvgScale(struct NVGcontext* ctx, float x, float y);"
 		"struct NVGpaint nvgLinearGradient(struct NVGcontext* ctx, float sx, float sy, float ex, float ey, struct NVGcolor icol, struct NVGcolor ocol);"
   		"struct NVGpaint nvgBoxGradient(struct NVGcontext* ctx, float x, float y, float w, float h, float r, float f, struct NVGcolor icol, struct NVGcolor ocol);"
	 	"struct NVGpaint nvgRadialGradient(struct NVGcontext* ctx, float cx, float cy, float inr, float outr, struct NVGcolor icol, struct NVGcolor ocol);"
   		"struct NVGpaint nvgImagePattern(struct NVGcontext* ctx, float ox, float oy, float ex, float ey, float angle, int image, float alpha);"
		"void nvgScissor(struct NVGcontext* ctx, float x, float y, float w, float h);"
		"void nvgIntersectScissor(struct NVGcontext* ctx, float x, float y, float w, float h);"
		"void nvgResetScissor(struct NVGcontext* ctx);"
		"void nvgBeginPath(struct NVGcontext* ctx);"
		"void nvgMoveTo(struct NVGcontext* ctx, float x, float y);"
		"void nvgLineTo(struct NVGcontext* ctx, float x, float y);"
		"void nvgBezierTo(struct NVGcontext* ctx, float c1x, float c1y, float c2x, float c2y, float x, float y);"
		"void nvgQuadTo(struct NVGcontext* ctx, float cx, float cy, float x, float y);"
		"void nvgArcTo(struct NVGcontext* ctx, float x1, float y1, float x2, float y2, float radius);"
		"void nvgClosePath(struct NVGcontext* ctx);"
		"void nvgPathWinding(struct NVGcontext* ctx, int dir);"
		"void nvgArc(struct NVGcontext* ctx, float cx, float cy, float r, float a0, float a1, int dir);"
		"void nvgRect(struct NVGcontext* ctx, float x, float y, float w, float h);"
		"void nvgRoundedRect(struct NVGcontext* ctx, float x, float y, float w, float h, float r);"
		"void nvgEllipse(struct NVGcontext* ctx, float cx, float cy, float rx, float ry);"
		"void nvgCircle(struct NVGcontext* ctx, float cx, float cy, float r);"
		"void nvgFill(struct NVGcontext* ctx);"
		"void nvgStroke(struct NVGcontext* ctx);"
		"void nvgFontSize(struct NVGcontext* ctx, float size);"
		"void nvgFontBlur(struct NVGcontext* ctx, float blur);"
		"void nvgTextLetterSpacing(struct NVGcontext* ctx, float spacing);"
		"void nvgTextLineHeight(struct NVGcontext* ctx, float lineHeight);"
		"void nvgTextAlign(struct NVGcontext* ctx, int align);"
  		"void nvgFontFace(struct NVGcontext* ctx, const char* font);]]", "internal: NanoVG");

	// TODO: Port nvgText, nvgTextBox, nvgTextBounds, nvgTextBoxBounds
	// TODO: nvgTextMetrics, nvgTextBreakLines
	// TODO: nvgCurrentTransform and similar functions which operate on float*
	// TODO: image stuff

	// Little macro to shorten a bit the code
#define EXPORT_FFI(ffi_name, table_name) table[table_name] = sview["ffi"]["C"][ffi_name]


	// Export the functions
	EXPORT_FFI("nvgSave", "save");
	EXPORT_FFI("nvgRestore", "restore");
	EXPORT_FFI("nvgStrokePaint", "stroke_paint");
	EXPORT_FFI("nvgFillPaint", "fill_paint");
	EXPORT_FFI("nvgStrokeWidth", "stroke_width");

	EXPORT_FFI("nvgResetTransform", "reset_transform");
	EXPORT_FFI("nvgTransform", "transform");
	EXPORT_FFI("nvgTranslate", "translate");
	EXPORT_FFI("nvgRotate", "rotate");
	EXPORT_FFI("nvgSkewX", "skew_x");
	EXPORT_FFI("nvgSkewY", "skew_y");
	EXPORT_FFI("nvgScale", "scale");

	EXPORT_FFI("nvgLinearGradient", "linear_gradient");
	EXPORT_FFI("nvgBoxGradient", "box_gradient");
	EXPORT_FFI("nvgRadialGradient", "radial_gradient");
	EXPORT_FFI("nvgImagePattern", "image_pattern");

	EXPORT_FFI("nvgScissor", "scissor");
	EXPORT_FFI("nvgIntersectScissor", "intersect_scissor");
	EXPORT_FFI("nvgResetScissor", "reset_scissor");

	EXPORT_FFI("nvgBeginPath", "begin_path");
	EXPORT_FFI("nvgMoveTo", "move_to");
	EXPORT_FFI("nvgLineTo", "line_to");
	EXPORT_FFI("nvgBezierTo", "bezier_to");
	EXPORT_FFI("nvgQuadTo", "quad_to");
	EXPORT_FFI("nvgArcTo", "arc_to");
	EXPORT_FFI("nvgClosePath", "close_path");

	EXPORT_FFI("nvgPathWinding", "path_winding");
	EXPORT_FFI("nvgArc", "arc");
	EXPORT_FFI("nvgRect", "rect");
	EXPORT_FFI("nvgRoundedRect", "rounded_rect");
	EXPORT_FFI("nvgEllipse", "ellipse");
	EXPORT_FFI("nvgCircle", "circle");

	EXPORT_FFI("nvgFill", "fill");
	EXPORT_FFI("nvgStroke", "stroke");

	EXPORT_FFI("nvgFontSize", "font_size");
	EXPORT_FFI("nvgFontBlur", "font_blur");
	EXPORT_FFI("nvgTextLetterSpacing", "text_letter_spacing");
	EXPORT_FFI("nvgTextLineHeight", "text_line_height");
	EXPORT_FFI("nvgTextAlign", "text_align");
	EXPORT_FFI("nvgFontFace", "font_face");

	// Unload ffi to avoid security risks
	sview["ffi"] = sol::nil;

	// For some reason, the color related functions dont work very well so
	// we implement them manually. This prevents runtime crashes
	table["rgb"] = [](unsigned char r, unsigned char g, unsigned char b)
	{
		return nvgRGB(r, g, b);
	};
	table["rgbf"] = [](float r, float g, float b)
	{
		return nvgRGBf(r, g, b);
	};
	table["rgba"] = [](unsigned char r, unsigned char g, unsigned char b, unsigned char a)
	{
		return nvgRGBA(r, g, b, a);
	};
	table["rgbaf"] = [](float r, float g, float b, float a)
	{
		return nvgRGBAf(r, g, b, a);
	};
	table["fill_color"] = [](void* vg, NVGcolor color)
	{
		nvgFillColor((NVGcontext*)vg, color);
	};
	table["stroke_color"] = [](void* vg, NVGcolor color)
	{
		nvgStrokeColor((NVGcontext*)vg, color);
	};
	table["lerp_rgba"] = [](NVGcolor a, NVGcolor b, float u)
	{
		return nvgLerpRGBA(a, b, u);
	};
	table["trans_rgba"] = [](NVGcolor c, unsigned char a)
	{
		return nvgTransRGBA(c, a);
	};
	table["trans_rgbaf"] = [](NVGcolor c, float a)
	{
		return nvgTransRGBAf(c, a);
	};
	table["hsl"] = [](float h, float s, float l)
	{
		return nvgHSL(h, s, l);
	};
	table["hsla"] = [](float h, float s, float l, unsigned char a)
	{
		return nvgHSLA(h, s, l, a);
	};

}
