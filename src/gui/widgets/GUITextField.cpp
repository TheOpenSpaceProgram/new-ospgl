#include "GUITextField.h"
#include <utf8/utf8.h>
#include "util/UTF8Util.h"

glm::ivec2 GUITextField::prepare(glm::ivec2 wpos, glm::ivec2 wsize, glm::ivec4 viewport, GUIInput* ipt)
{
	pos = wpos;
	size = wsize;
	if(size.x < 0)
	{
		size.x = wsize.x;
	}

	if(size.y < 0)
	{
		size.y = wsize.y;
	}


	if(ipt->ext_mouse_blocked)
	{
		focused = false;
	}
	else
	{
		if(ipt->mouse_down(0))
		{
			if(ipt->mouse_inside(pos, size) && ipt->mouse_inside(viewport))
			{
				if(blocks_mouse)
				{
					ipt->mouse_blocked = true;
				}

				// Clear the input
				input->get_input_text();
				focused = true;
				ipt->keyboard_blocked = true;

				// Focus where the user clicked
				if(!glyph_pos.empty())
				{
					for (size_t glyph = 0; glyph < glyph_pos.size(); glyph++)
					{
						float left = glyph_pos[glyph].minx - last_offset;
						float right = glyph_pos[glyph].maxx - last_offset;

						if (input->mouse_pos.x > left && input->mouse_pos.y < right)
						{
							cursor_pos = (int) glyph;
						}
					}
					float minx = glyph_pos[0].minx - last_offset;
					float maxx = glyph_pos[glyph_pos.size() - 1].maxx - last_offset;
					if (input->mouse_pos.x > maxx)
					{
						cursor_pos = glyph_pos.size();
					}
					else if(input->mouse_pos.x < minx)
					{
						cursor_pos = 0;
					}
				}
			}
			else
			{
				if(focused)
				{
					ipt->keyboard_blocked = false;
				}
				focused = false;
			}
		}
	}

	if(focused && ipt->execute_user_actions)
	{
		std::string in = input->get_input_text();

		if(input->key_down_or_repeating(GLFW_KEY_RIGHT))
		{
			cursor_pos++;
			if(cursor_pos >= (int)glyph_pos.size())
			{
				cursor_pos = (int)glyph_pos.size();
			}
		}
		if(input->key_down_or_repeating(GLFW_KEY_LEFT))
		{
			cursor_pos--;
			if(cursor_pos < 0)
			{
				cursor_pos = 0;
			}
		}

		if(input->key_down_or_repeating(GLFW_KEY_BACKSPACE))
		{
			if(!string.empty())
			{
				auto it = string.begin();
				auto prev = string.begin();
				for (int i = 0; i < cursor_pos; i++)
				{
					prev = it;
					utf8::next(it, string.end());
				}
				// This way we erase the whole codepoint, not just a byte
				string.erase(prev, it);
				cursor_pos--;

				on_change.call(std::move(string));
			}
		}
		else if(!in.empty())
		{
			auto it = string.begin();
			for (int i = 0; i < cursor_pos; i++)
			{
				utf8::next(it, string.end());
			}

			string.insert(it, in.begin(), in.end());
			// Note that it may in some case be multiple codepoints, for example
			// if the user writes faster than the framerate!
			cursor_pos += UTF8Util::count_codepoints(in);

			on_change.call(std::move(string));
		}

		if(input->key_down(GLFW_KEY_ESCAPE))
		{
			focused = false;
			ipt->keyboard_blocked = false;
			on_exit.call(std::move(string));
		}

		if(input->key_down(GLFW_KEY_ENTER))
		{
			focused = false;
			ipt->keyboard_blocked = false;
			on_intro.call(std::move(string));
		}
	}

	return size;
}

void GUITextField::draw(NVGcontext* vg, GUISkin* skin)
{
	// TODO: Maybe write a new skin function for text fields?
	skin->draw_button(vg, pos, size, "", GUISkin::ButtonState::NORMAL, GUISkin::ButtonStyle::SYMMETRIC);

	if(uses_bitmap)
	{
		AssetHandle<BitmapFont> bfont = AssetHandle<BitmapFont>("core:fonts/ProggySquare.fnt");
		nvgBitmapText(vg, bfont.duplicate(), 0, pos.x, pos.y, string.c_str());
		// TODO: Write this code
	}
	else
	{
		nvgScissor(vg, pos.x, pos.y, size.x, size.y);

		nvgFontSize(vg, ft_size);
		nvgFontFace(vg, ft_font.c_str());
		float asc, desc, lh, lwidth = size.x - 10.0f;
		nvgTextMetrics(vg, &asc, &desc, &lh);
		nvgTextAlign(vg, NVG_ALIGN_MIDDLE);

		if(string.empty() && !focused)
		{
			nvgFillColor(vg, skin->get_foreground_color(true));
			nvgText(vg, pos.x + 2.0f, pos.y + size.y * 0.5f + 2.0f, default_string.c_str(), nullptr);
		}
		else
		{
			nvgFillColor(vg, skin->get_foreground_color());
			// This is unicode compatible, so glyphs may not exactly match string.size!
			glyph_pos.resize(string.size());
			if(!string.empty())
			{
				int glyph_count = nvgTextGlyphPositions(vg, pos.x, pos.y, string.c_str(), nullptr,
														&glyph_pos[0], string.size());
				glyph_pos.resize(glyph_count);
			}

			float cursor_x = 0.0f;

			// Note that the cursor is drawn at the "left" (x) of the
			// cursor_pos pointed glyph, so the last character is "virtual"
			if(glyph_pos.empty())
			{
				cursor_x = pos.x;
			}
			else
			{
				if (cursor_pos >= glyph_pos.size())
				{
					cursor_x = glyph_pos[glyph_pos.size() - 1].maxx;
				}
				else if (cursor_pos < 0)
				{
					cursor_x = glyph_pos[0].x;
				}
				else
				{
					cursor_x = glyph_pos[cursor_pos].x;
				}
			}

			float offset = 0.0f;
			if(cursor_x > pos.x + size.x)
			{
				offset = cursor_x - pos.x - size.x + 4.0f;
			}
			last_offset = offset;

			nvgText(vg, pos.x + 2.0f - offset, pos.y + size.y * 0.5f + 2.0f, string.c_str(), nullptr);
			double time = glfwGetTime();
			double timemod = fmod(time, 1.0);
			if(timemod > 0.5 && focused)
			{
				nvgStrokeColor(vg, skin->get_foreground_color());
				nvgBeginPath(vg);
				nvgMoveTo(vg, cursor_x + 1.0f - offset, pos.y + 3.0f);
				nvgLineTo(vg, cursor_x + 1.0f - offset, pos.y + size.y - 3.0f);
				nvgStroke(vg);
			}
		}

		nvgResetScissor(vg);
	}
}

GUITextField::GUITextField(std::string font, float size)
{
	focused = false;
	string = "";
	default_string = "";
	uses_bitmap = false;
	cursor_pos = 0;
	ft_size = size;
	ft_font = font;
}
