#include "SimpleSkin.h"

void SimpleSkin::draw_button(NVGcontext* v, glm::ivec2 pos, glm::ivec2 size, const std::string& text,
	ButtonState state, ButtonStyle style)
{
	
	nvgBeginPath(v);	

	int bevel_pixels = 10;
	
	if(size.x <= 32 || size.y <= 32)
	{
		// Fallback to a rectangle, bevel looks wrong
		nvgRect(v, pos.x - 0.5f, pos.y - 0.5f, size.x + 1.0f, size.y + 1.0f);	
	}
	else
	{
	
		if(style == ButtonStyle::NORMAL)
		{
			nvgMoveTo(v, pos.x + 0.5f, pos.y + size.y + 0.5f);
			nvgLineTo(v, pos.x + size.x - bevel_pixels + 0.5f, pos.y + size.y + 0.5f);
			nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + size.y - bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + 0.5f);
			nvgLineTo(v, pos.x + bevel_pixels + 0.5f, pos.y + 0.5f);
			nvgLineTo(v, pos.x + 0.5f, pos.y + bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + 0.5f, pos.y + size.y + 0.5f);	
		}
		else if(style == ButtonStyle::SYMMETRIC)
		{
			// Similar to before but with "bevel" on all borders
			nvgMoveTo(v, pos.x + bevel_pixels + 0.5f, pos.y + 0.5f);
			nvgLineTo(v, pos.x + 0.5f, pos.y + bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + 0.5f, pos.y + size.y - bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + bevel_pixels + 0.5f, pos.y + size.y + 0.5f);
			nvgLineTo(v, pos.x + size.x - bevel_pixels + 0.5f, pos.y + size.y + 0.5f);
			nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + size.y - bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + size.x + 0.5f, pos.y + bevel_pixels + 0.5f);
			nvgLineTo(v, pos.x + size.x - bevel_pixels + 0.5f, pos.y + 0.5f);
			nvgLineTo(v, pos.x + bevel_pixels + 0.5f, pos.y + 0.5f);	
		}
	}

	nvgStrokeWidth(v, 1.0f);

	if(style == ButtonStyle::GOOD)
	{
	}
	else if(style == ButtonStyle::BAD)
	{
		
	}
	else
	{
		nvgFillColor(v, nvgRGB(21, 21, 21));
		nvgStrokeColor(v, nvgRGB(73, 73, 73));
	}

	nvgFill(v);
	nvgStroke(v);

}	
