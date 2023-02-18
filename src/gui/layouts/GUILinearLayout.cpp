#include "GUILinearLayout.h"
#include <gui/GUIScreen.h>

void GUILinearLayout::on_remove_widget(GUIWidget *widget)
{
	for(size_t i = 0; i < widgets.size(); i++)
	{
		if(widgets[i].get() != widget)
			continue;

		same_line.erase(same_line.begin() + i);

		break;
	}
}

void GUILinearLayout::on_move_widget(GUIWidget *widget, int new_index)
{
	// TODO: Update same_line
}

void GUILinearLayout::on_add_widget(GUIWidget *widget)
{
	same_line.push_back(false);
}

void GUILinearLayout::mark_same_line()
{
	same_line[same_line.size() - 1] = true;
}

void GUILinearLayout::linear_helper(glm::ivec2 vpos, glm::ivec2 vsize, GUIScreen *screen, bool vertical)
{
	this->pos = vpos;
	this->size = vsize;

	vpos += glm::ivec2(margins.x, margins.z);
	vsize -= glm::ivec2(margins.x + margins.y, margins.z + margins.w);
	if(vertical)
		vsize.x -= vscrollbar.get_width(screen->skin.get());
	else
		vsize.y -= hscrollbar.get_width(screen->skin.get());

	int base_pos;
	int sameline_pos;

	if(vertical)
	{
		sameline_pos = vpos.x;
		base_pos = vpos.y - vscrollbar.scroll;
	}
	else
	{
		sameline_pos = vpos.y;
		base_pos = vpos.x - hscrollbar.scroll;
	}

	int in_same_line = 0;
	float remainders = 0;
	bool last_same_line = false;

	for(size_t i = 0; i < widgets.size(); i++)
	{
		bool jump = false;
		auto widget = widgets[i];
		// Check for same-line starting now
		if(same_line[i])
		{
			if(in_same_line == 0)
			{
				// Count how many elements in same line are coming
				size_t j = i;
				while(j < widgets.size())
				{
					if(!same_line[j])
					{
						break;
					}

					in_same_line++;
					j++;
				}
			}
			last_same_line = true;
		}
		else
		{
			if(!last_same_line)
			{
				in_same_line = 0;
				if(vertical)
					sameline_pos = vpos.x;
				else
					sameline_pos = vpos.y;
			}
			last_same_line = false;
			jump = true;
		}

		glm::ivec2 pos;
		if(vertical)
			pos = glm::ivec2(sameline_pos, base_pos);
		else
			pos = glm::ivec2(base_pos, sameline_pos);

		glm::ivec2 size;
		if(vertical)
			size = glm::ivec2(vsize.x, -1.0);
		else
			size = glm::ivec2(-1.0, vsize.y);

		if(in_same_line > 0)
		{
			// Divide horizontal size
			float n_size;
			if(vertical)
			{
				n_size = (float) size.x / (float) (in_same_line + 1);
				size.x = floor(n_size);
			}
			else
			{
				n_size = (float) size.y / (float) (in_same_line + 1);
				size.y = floor(n_size);
			}

			remainders += n_size - floor(n_size);
			if(jump)
			{
				if(vertical)
					size.x += remainders;
				else
					size.y += remainders;

				remainders = 0;
			}
		}

		glm::ivec2 used = widget->position(pos, size, screen);

		// Culling
		if(vertical)
			widget->is_visible = !(base_pos - vpos.y > vsize.y || base_pos - vpos.y < -used.y);
		else
			widget->is_visible = !(base_pos - vpos.x > vsize.x || base_pos - vpos.x < -used.x);


		if(in_same_line > 0 && !jump)
		{
			if(vertical)
				sameline_pos += size.x;
			else
				sameline_pos += size.y;
		}
		else
		{
			if(vertical)
			{
				base_pos += used.y + element_margin;
				sameline_pos = vpos.x;
			}
			else
			{
				base_pos += used.x + element_margin;
				sameline_pos = vpos.y;
			}
		}


	}

	if(vertical)
		vscrollbar.max_scroll = base_pos - vpos.y + vscrollbar.scroll;
	else
		hscrollbar.max_scroll = base_pos - vpos.x + hscrollbar.scroll;

}

