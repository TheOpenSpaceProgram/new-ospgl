#include "PlumbingEditor.h"
#include <lua/libs/LuaNanoVG.h>

void PlumbingEditor::show_editor(NVGcontext* vg, GUIInput* gui_input, glm::vec4 span)
{
	if(veh == nullptr)
	{
		logger->warn("Plumbing editor shown without assigned vehicle");
		return;
	}

	bool block = false;
	hovered = PlumbingElement();
	block |= update_mouse(gui_input, span);
	block |= update_pipes(gui_input, span);
	block |= update_selection(gui_input, span);

	if(block)
	{
		gui_input->ext_mouse_blocked = true;
		gui_input->ext_scroll_blocked = true;
		gui_input->mouse_blocked = true;
		gui_input->scroll_blocked = true;
	}

	nvgSave(vg);
	nvgScissor(vg, span.x, span.y, span.z, span.w);

	nvgStrokeColor(vg, nvgRGB(230, 230, 230));
	draw_grid(vg, span);
	draw_machines(vg, span);
	draw_junctions(vg, span);
	draw_pipes(vg, span);
	draw_selection(vg, span);
	draw_collisions(vg, span);

	nvgRestore(vg);
}

PlumbingEditor::PlumbingEditor()
{
	cam_center = glm::vec2(0.0f, 0.0f);
	zoom = 32;
	veh = nullptr;
	in_drag = false;
	in_selection = false;
	in_pipe_drag = false;
	vacant_junction = nullptr;
}

void PlumbingEditor::draw_grid(NVGcontext* vg, glm::vec4 span) const
{

	nvgBeginPath(vg);

	int max_halfx = (int)((span.z / 2.0f) / (float)zoom);
	int max_halfy = (int)((span.w / 2.0f) / (float)zoom);

	nvgResetTransform(vg);
	glm::vec2 center = glm::vec2(span.x + span.z * 0.5f, span.y + span.w * 0.5f);
	nvgTranslate(vg, center.x-fmod(cam_center.x, 1.0f) * zoom, center.y-fmod(cam_center.y, 1.0f) * zoom);
	nvgScale(vg, (float)zoom, (float)zoom);
	nvgStrokeWidth(vg, 1.0f / (float)zoom);
	// Vertical lines
	for(int x = -max_halfx - 1; x < max_halfx + 1; x++)
	{
		nvgMoveTo(vg, (float)x, -(float)max_halfy - 1);
		nvgLineTo(vg, (float)x, (float)max_halfy + 1);
	}

	for(int y = -max_halfy - 1; y < max_halfy + 1; y++)
	{
		nvgMoveTo(vg, -(float)max_halfx - 1, (float)y);
		nvgLineTo(vg, (float)max_halfx + 1, (float)y);
	}
	nvgStroke(vg);
}


void PlumbingEditor::draw_machines(NVGcontext* vg, glm::vec4 span) const
{
	nvgResetTransform(vg);

	glm::vec2 center = glm::vec2(span.x + span.z * 0.5f, span.y + span.w * 0.5f);

	int i = 0;

	for(const Part* p : veh->parts)
	{
		for(const auto& pair : p->machines)
		{
			if(pair.second->plumbing.has_lua_plumbing())
			{
				bool is_selected = false;
				bool is_conflict = false;
				for(PlumbingElement elem : selected)
				{
					if(elem == pair.second)
					{
						is_selected = true;
						break;
					}
				}

				for(PlumbingElement elem : drag_conflicts)
				{
					if(elem == pair.second)
					{
						is_conflict = true;
						break;
					}
				}

				nvgStrokeWidth(vg, 2.0f / (float)zoom);
				nvgStrokeColor(vg, nvgRGB(0, 0, 0));
				nvgFillColor(vg, nvgRGB(255, 255, 255));

				if(hovered == pair.second || is_selected)
				{
					nvgStrokeWidth(vg, 4.0f / (float)zoom);
				}
				if(is_selected)
				{
					nvgFillColor(vg, nvgRGB(128, 128, 255));
				}

				if(is_conflict)
				{
					nvgStrokeColor(vg, nvgRGB(255, 0, 0));
				}

				nvgResetTransform(vg);
				glm::vec2 ppos = pair.second->plumbing.editor_position;

				// Preview drag
				if(in_machine_drag && is_selected)
				{
					glm::vec2 offset = mouse_current - mouse_start;
					ppos += glm::round(offset);
				}

				nvgTranslate(vg, center.x + (ppos.x - cam_center.x) * (float)zoom,
				 	center.y + (ppos.y - cam_center.y) * (float)zoom);
				nvgScale(vg, (float)zoom, (float)zoom);
				glm::ivec2 size = pair.second->plumbing.get_editor_size(false, false);
				// The logical rotation simply changes x and y dimensions, which is similar
				// to tipping over the box
				if(pair.second->plumbing.editor_rotation == 1)
				{
					nvgTranslate(vg, (float)size.y, 0.0f);
				}
				else if(pair.second->plumbing.editor_rotation == 2)
				{
					nvgTranslate(vg, (float)size.x, (float)size.y);
				}
				else if(pair.second->plumbing.editor_rotation == 3)
				{
					nvgTranslate(vg, 0.0f, (float)size.x);
				}
				nvgRotate(vg, glm::half_pi<float>() * (float)pair.second->plumbing.editor_rotation);

				pair.second->plumbing.draw_diagram((void*)vg);

				// Draw the ports
				for(const FluidPort& port : pair.second->plumbing.fluid_ports)
				{
					bool hover_port = hovering_port == port.id && hovered == pair.second;
					draw_port(vg, port.pos, hover_port);
				}
			}
		}
		i += 3;
	}

}

bool PlumbingEditor::update_mouse(GUIInput* gui_input, glm::vec4 span)
{
	bool block = false;
	bool inside_and_not_blocked = is_inside_and_not_blocked(gui_input, span);

	// Click and drag
	if(inside_and_not_blocked || in_drag)
	{
		if (gui_input->mouse_down(1))
		{
			last_click = input->mouse_pos;
			last_center = cam_center;
			in_drag = true;
		}

		if(gui_input->mouse_up(1))
		{
			in_drag = false;
		}

		if(gui_input->mouse_pressed(1))
		{
			glm::vec2 delta = input->mouse_pos - last_click;
			cam_center = last_center - delta / (float)zoom;
			block = true;
		}

	}

	// Zooming in and out
	if(inside_and_not_blocked)
	{
		block = true;
		double delta = input->mouse_scroll_delta;
		zoom += zoom * delta * 0.1;
		zoom = glm::clamp(zoom, 24, 80);
	}

	return block;
}

bool PlumbingEditor::update_dragging(GUIInput *gui_input, glm::vec2 mpos)
{
	if(in_machine_drag)
	{
		mouse_current = mpos;
		glm::vec2 offset = glm::round(mouse_current - mouse_start);

		int prev_rotation = -1;
		double time_passed = glfwGetTime() - time_held;
		// Rotation
		if(input->mouse_up(0) && offset == glm::vec2(0, 0) && selected.size() == 1
			&& time_passed < max_time_for_rotation)
		{
			prev_rotation = selected[0].get_rotation();
			selected[0].set_rotation(selected[0].get_rotation() + 1);
			if(selected[0].get_rotation() == 4)
			{
				selected[0].set_rotation(0);
			}
		}

		drag_conflicts.clear();
		for(PlumbingElement elem : selected)
		{
			glm::ivec2 end_pos = elem.get_pos() + glm::ivec2(offset);
			if(!veh->plumbing.grid_aabb_check(end_pos,end_pos + elem.get_size(true),
											  selected, true).empty())
			{
				drag_conflicts.push_back(elem);
			}
		}


		if(input->mouse_up(0))
		{
			if(drag_conflicts.empty())
			{
				for (PlumbingElement elem : selected)
				{
					elem.set_pos(elem.get_pos() + glm::ivec2(offset));
				}
			}

			in_machine_drag = false;
			if(prev_rotation != -1 && !drag_conflicts.empty())
			{
				selected[0].set_rotation(prev_rotation);
			}
			drag_conflicts.clear();
		}
		return true;
	}
	else if(!in_pipe_drag)
	{
		bool hovered_selected = false;
		for (PlumbingElement elem : selected)
		{
			if (elem == hovered)
			{
				hovered_selected = true;
				break;
			}
		}

		if (gui_input->mouse_down(0) && hovered_selected)
		{
			in_machine_drag = true;
			time_held = glfwGetTime();
			mouse_start = mpos;
			mouse_current = mpos;
			return true;
		}

		return false;
	}

	return false;
}

void PlumbingEditor::handle_hovering(GUIInput *gui_input, glm::vec2 mpos)
{
	// First check if we are hovering any port

	auto hover_vec = veh->plumbing.grid_aabb_check(mpos, mpos);
	if (hover_vec.size() >= 1)
	{
		PlumbingElement elem = hover_vec[0];
		hovered = elem;
		if (gui_input->mouse_down(2) && elem.type == PlumbingElement::MACHINE)
		{
			on_middle_click(elem.as_machine);
		}
	}

	// Go over every port and check if it's hovered

}

bool PlumbingEditor::update_selection(GUIInput *gui_input, glm::vec4 span)
{
	bool block = false;
	bool dragged = false;
	bool inside_and_not_blocked = is_inside_and_not_blocked(gui_input, span);

	glm::vec2 mpos = get_mouse_pos(span);

	// Hovering
	if(inside_and_not_blocked)
	{
		handle_hovering(gui_input, mpos);
	}

	if(!in_selection && inside_and_not_blocked && !in_drag || in_machine_drag)
	{
		dragged = update_dragging(gui_input, mpos);
		block |= dragged;
	}

	// Left click + drag does box selection and unselects everything else
	// Shift + Left click adds to selection with box

	if(!in_drag && !in_selection && inside_and_not_blocked && !dragged && !in_pipe_drag)
	{
		if(gui_input->mouse_down(0))
		{
			// Check if we are over a port or a pipe, this will generate a new pipe

			in_selection = true;
			mouse_start = mpos;
		}
	}

	if(in_selection)
	{
		mouse_current = mpos;
		if(gui_input->mouse_up(0))
		{
			if(!input->key_pressed(GLFW_KEY_LEFT_SHIFT) && !input->key_pressed(GLFW_KEY_RIGHT_SHIFT))
			{
				selected.clear();
			}

			// AABB check
			if(mouse_start.x > mouse_current.x)
			{
				std::swap(mouse_start.x, mouse_current.x);
			}
			if(mouse_start.y > mouse_current.y)
			{
				std::swap(mouse_start.y, mouse_current.y);
			}
			auto new_sel = veh->plumbing.grid_aabb_check(mouse_start, mouse_current, selected);
			selected.insert(selected.end() , new_sel.begin(), new_sel.end());

			in_selection = false;
		}
	}

	return block;
}

bool PlumbingEditor::update_pipes(GUIInput *gui_input, glm::vec4 span)
{
	glm::vec2 mpos = get_mouse_pos(span);
	glm::ivec2 round = glm::floor(mpos);
	hovering_port = "";
	hovering_port_user = nullptr;
	// Hovering ports
	std::vector<PlumbingElement> all_elems = veh->plumbing.get_all_elements();
	for (PlumbingElement &elem : all_elems)
	{
		// We may be hovering a junction (as a whole, not ports) too
		if(elem.type == PlumbingElement::JUNCTION &&
			elem.get_pos().x <= round.x && elem.get_pos().y <= round.y &&
			elem.get_pos().x + elem.get_size().x > round.x &&
			elem.get_pos().y + elem.get_size().y > round.y)
		{
			hovered = elem;
		}

		auto ports = elem.get_ports();
		for (auto pair : ports)
		{
			// If in pipe drag we just need to be in the same square to hover the port
			if ((!in_pipe_drag && glm::distance(mpos, pair.second) <= port_radius * 1.15f) ||
					(in_pipe_drag && round == (glm::ivec2)glm::floor(pair.second)))
			{
				if(elem.type == PlumbingElement::MACHINE)
				{
					for (Pipe &p : veh->plumbing.pipes)
					{
						if ((elem == p.ma && p.port_a == pair.first.id) ||
							(elem == p.mb && p.port_b == pair.first.id))
						{
							hovering_port_user = &p;
							break;
						}
					}
				}
				else if(elem.type == PlumbingElement::JUNCTION)
				{
					for(Pipe* p : elem.as_junction->pipes)
					{
						// TODO: This check feels like a workaround, use some kind of id??
						if((glm::ivec2)glm::floor(elem.as_junction->get_port_position(p)) == round)
						{
							hovering_port_user = p;
							break;
						}
					}

				}

				hovering_port = pair.first.id;
				hovered = elem;
				break;
			}
		}
	}

	if(in_pipe_drag)
	{
		if(gui_input->mouse_down(0))
		{
			if(hovered.type == PlumbingElement::JUNCTION)
			{
				hovering_pipe->invert();
				hovering_pipe->connect_junction(hovered.as_junction);
				in_pipe_drag = false;
				hovering_pipe = nullptr;
			}
			else if(!hovering_port.empty())
			{
				// End a pipe in a port
				handle_vacant_junction();
				in_pipe_drag = false;
				hovering_pipe->mb = hovered.as_machine;
				hovering_pipe->port_b = hovering_port;
				hovering_pipe = nullptr;
			}
			else
			{
				// Add a waypoint at mouse pos, remove it if it's already there, or create junction if other pipe contains it
				auto it = std::find(hovering_pipe->waypoints.begin(), hovering_pipe->waypoints.end(), round);
				if(it == hovering_pipe->waypoints.end())
				{
					// try to find it in another pipe
					// We cannot use pointers here!
					size_t found_in_id = 0;
					size_t waypoint_i = 0;
					for(Pipe& p : veh->plumbing.pipes)
					{
						for(size_t i = 0; i < p.waypoints.size(); i++)
						{
							if (p.waypoints[i] == round)
							{
								found_in_id = p.id;
								waypoint_i = i;
								break;
							}
						}
						// Early exit
						if(found_in_id != 0)
						{
							break;
						}
					}

					if(found_in_id != 0)
					{
						// Pointers will be invalidated!
						size_t hovering_id = hovering_pipe->id;
						// Create a junction, if it fits, between the three new pipes
						PipeJunction* jnc = veh->plumbing.create_pipe_junction();
						Pipe* n_pipe = veh->plumbing.create_pipe();
						// Restore pointers
						Pipe* found_in = veh->plumbing.get_pipe(found_in_id);
						hovering_pipe = veh->plumbing.get_pipe(hovering_id);
						jnc->pos = found_in->waypoints[waypoint_i];
						// Connect the new pipe to the old target and the junction
						n_pipe->mb = found_in->mb;
						n_pipe->port_b = found_in->port_b;
						n_pipe->connect_junction(jnc);
						// The new pipe gets all waypoints FROM the overriden one
						n_pipe->waypoints.insert(n_pipe->waypoints.begin(),
												 found_in->waypoints.begin() + waypoint_i + 1, found_in->waypoints.end());
						// The old pipe gets all waypoints up to the overriden one
						found_in->waypoints.erase(found_in->waypoints.begin() + waypoint_i, found_in->waypoints.end());
						// Disconnect the old pipe and connect it to the junction
						found_in->invert();
						found_in->connect_junction(jnc);
						// Connect ourselves to the junction, we must also invert
						hovering_pipe->invert();
						hovering_pipe->connect_junction(jnc);

						hovering_pipe = nullptr;
						in_pipe_drag = false;
					}
					else
					{
						hovering_pipe->waypoints.push_back(round);
					}
				}
				else
				{
					hovering_pipe->waypoints.erase(it);
				}
			}
		}
	}
	else
	{
		// Creating new pipes, finishing unfinished ones, or modifying them
		if (!hovering_port.empty() && gui_input->mouse_down(0))
		{
			if(hovering_port_user)
			{
				// Modify existing pipe, multiple conditions
				hovering_pipe = hovering_port_user;
				in_pipe_drag = true;
				if(hovered == hovering_port_user->ma)
				{
					// Modify from the start (invert pipe)
					hovering_pipe->invert();
				}

				if(hovered.type == PlumbingElement::JUNCTION)
				{
					// We leave a vacant pipe slot in the junction to avoid it from disappearing
					// It will disappear if the pipe is saved and the vacant slot is not filled
					// A vacant slot is simply a nullptr in the array
					PipeJunction* jnc = hovered.as_junction;
					// Disconnect the junction
					hovering_pipe->junction = nullptr;
					hovering_pipe->junction_id = 0;
					// Invert the pipe
					hovering_pipe->invert();
					// Now remove the pipe from the junction, leaving a vacant slot
					for(size_t i = 0; i < jnc->pipes.size(); i++)
					{
						if(jnc->pipes[i] == hovering_pipe)
						{
							jnc->pipes[i] = nullptr;
							jnc->pipes_id[i] = 0xDEADBEEF;
							break;
						}
					}

					vacant_junction = jnc;

				}
				else
				{
					// Disconnect the pipe
					hovering_pipe->mb = nullptr;
					hovering_pipe->port_b = "";
				}
			}
			else
			{
				// Create new pipe
				Pipe* n_pipe = veh->plumbing.create_pipe();
				n_pipe->ma = hovered.as_machine;
				n_pipe->port_a = hovering_port;

				hovering_pipe = n_pipe;
				in_pipe_drag = true;
			}
		}
	}

	return false;
}

bool PlumbingEditor::is_inside_and_not_blocked(GUIInput *gui_input, glm::vec4 span)
{
	return gui_input->mouse_inside(span) &&
		!gui_input->mouse_blocked && !gui_input->ext_mouse_blocked &&
		!gui_input->scroll_blocked && !gui_input->ext_scroll_blocked;
}


glm::vec2 PlumbingEditor::get_mouse_pos(glm::vec4 span) const
{
	glm::vec2 window_relative = input->mouse_pos;
	glm::vec2 span_relative;
	span_relative.x = (window_relative.x - span.x) / span.z;
	span_relative.y = (window_relative.y - span.y) / span.w;
	glm::vec2 center_relative = span_relative - glm::vec2(0.5f, 0.5f);
	center_relative.x *= span.z;
	center_relative.y *= span.w;

	return cam_center + center_relative / (float)zoom;

}

void PlumbingEditor::draw_selection(NVGcontext *vg, glm::vec4 span) const
{
	if(in_selection)
	{
		nvgResetTransform(vg);
		glm::vec2 center = glm::vec2(span.x + span.z * 0.5f, span.y + span.w * 0.5f);
		nvgTranslate(vg, center.x - cam_center.x * (float) zoom, center.y - cam_center.y * (float) zoom);
		nvgScale(vg, (float) zoom, (float) zoom);

		nvgStrokeColor(vg, nvgRGB(0, 0, 255));
		nvgFillColor(vg, nvgRGBA(0, 0, 255, 100));
		nvgStrokeWidth(vg, 1.0f / (float)zoom);

		nvgBeginPath(vg);
		nvgRect(vg, mouse_start.x, mouse_start.y,
				mouse_current.x - mouse_start.x, mouse_current.y - mouse_start.y);
		nvgStroke(vg);
		nvgFill(vg);
	}
}

void PlumbingEditor::draw_pipe_cap(NVGcontext *vg, glm::vec2 pos) const
{
	float r = port_radius / sqrtf(2.0f);
	nvgMoveTo(vg, pos.x, pos.y);
	nvgLineTo(vg, pos.x + r, pos.y + r);
	nvgMoveTo(vg, pos.x, pos.y);
	nvgLineTo(vg, pos.x - r, pos.y - r);
	nvgMoveTo(vg, pos.x, pos.y);
	nvgLineTo(vg, pos.x - r, pos.y + r);
	nvgMoveTo(vg, pos.x, pos.y);
	nvgLineTo(vg, pos.x + r, pos.y - r);
}

static bool test_test = false;

void PlumbingEditor::draw_pipes(NVGcontext *vg, glm::vec4 span) const
{
	// TODO: Add a guard for non interactive editor?
	glm::vec2 mpos = get_mouse_pos(span);

	nvgResetTransform(vg);
	glm::vec2 center = glm::vec2(span.x + span.z * 0.5f, span.y + span.w * 0.5f);
	nvgTranslate(vg, center.x - cam_center.x * (float) zoom, center.y - cam_center.y * (float) zoom);
	nvgScale(vg, (float) zoom, (float) zoom);

	for(const Pipe& p : veh->plumbing.pipes)
	{
		nvgStrokeColor(vg, nvgRGB(0, 0, 0));
		float size = 1.0f;
		// If we hover a port, the joined pipe gets in bold
		if(hovering_port_user == &p)
		{
			size = 2.0f;
		}
		nvgStrokeWidth(vg, size / (float)zoom);
		nvgBeginPath(vg);

		glm::vec2 pos;

		// We start on the first port, or first waypoint if such is not present
		if(p.ma != nullptr)
		{
			pos = p.ma->plumbing.get_port_position(p.port_a);
			if(in_machine_drag && std::find(selected.begin(), selected.end(), PlumbingElement(p.ma)) != selected.end())
			{
				pos += glm::round(mpos - mouse_start);
			}
		}
		else if(p.junction != nullptr)
		{
			pos = p.junction->get_port_position(&p);
			if(in_machine_drag && std::find(selected.begin(), selected.end(), PlumbingElement(p.junction)) != selected.end())
			{
				pos += glm::round(mpos - mouse_start);
			}
		}
		else
		{
			pos = (glm::vec2)p.waypoints[0] + glm::vec2(0.5f);
		}

		draw_pipe_cap(vg, pos);
		nvgMoveTo(vg, pos.x, pos.y);

		for(size_t i = 0; i < p.waypoints.size(); i++)
		{
			nvgLineTo(vg, p.waypoints[i].x + 0.5f, p.waypoints[i].y + 0.5f);
		}

		glm::vec2 end_pos;
		if(p.mb != nullptr)
		{
			end_pos = p.mb->plumbing.get_port_position(p.port_b);
			if(in_machine_drag && std::find(selected.begin(), selected.end(), PlumbingElement(p.mb)) != selected.end())
			{
				end_pos += glm::round(mpos - mouse_start);
			}
		}
		else if(in_pipe_drag && hovering_pipe == &p)
		{
			end_pos = mpos;
		}
		else
		{
			// An unfinished pipe must have an end waypoint
			end_pos = p.waypoints[p.waypoints.size() - 1];
		}

		nvgLineTo(vg, end_pos.x, end_pos.y);
		draw_pipe_cap(vg, end_pos);
		nvgStroke(vg);


		// While dragging, waypoints must be visible to allow modification or junction creation
		if(in_pipe_drag)
		{
			if(&p == hovering_pipe)
			{
				nvgFillColor(vg, nvgRGB(0, 0, 0));
				nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 0));
			}
			else
			{
				nvgFillColor(vg, nvgRGB(255, 255, 255));
				nvgStrokeColor(vg, nvgRGBA(0, 0, 0, 255));
			}

			for(size_t i = 0; i < p.waypoints.size(); i++)
			{
				// Draw waypoints as dots so the user knows where they are
				nvgBeginPath(vg);
				nvgCircle(vg, p.waypoints[i].x + 0.5f, p.waypoints[i].y + 0.5f, 4.0f / (float)zoom);
				nvgFill(vg);
				nvgStroke(vg);
			}
		}
	}

}

void PlumbingEditor::draw_junctions(NVGcontext *vg, glm::vec4 span) const
{
	nvgResetTransform(vg);
	glm::vec2 center = glm::vec2(span.x + span.z * 0.5f, span.y + span.w * 0.5f);
	nvgTranslate(vg, center.x - cam_center.x * (float) zoom, center.y - cam_center.y * (float) zoom);
	nvgScale(vg, (float) zoom, (float) zoom);

	for(const PipeJunction& j : veh->plumbing.junctions)
	{
		glm::vec2 pos = (glm::vec2)j.pos + glm::vec2(0.5f);
		bool contains = false;
		for(auto& pelem : selected)
		{
			if(pelem == &j)
			{
				contains = true;
				break;
			}
		}
		if(contains)
		{
			nvgStrokeColor(vg, nvgRGB(0, 0, 255));
			nvgStrokeWidth(vg, 2.0f / (float)zoom);
			if(in_machine_drag)
			{
				pos += glm::round(mouse_current - mouse_start);
			}
		}
		else
		{
			nvgStrokeColor(vg, nvgRGB(0, 0, 0));
			nvgStrokeWidth(vg, 1.0f / (float)zoom);
		}

		if(hovered == &j)
		{
			nvgStrokeWidth(vg, 2.0f / (float)zoom);
		}

		draw_junction(vg, pos, j.pipes.size());
	}
}

void PlumbingEditor::draw_collisions(NVGcontext* vg, glm::vec4 span) const
{
	if (in_machine_drag && !drag_conflicts.empty())
	{
		glm::vec2 center = glm::vec2(span.x + span.z * 0.5f, span.y + span.w * 0.5f);

		int i = 0;

		for(const Part* p : veh->parts)
		{
			for(const auto& pair : p->machines)
			{
				if(pair.second->plumbing.has_lua_plumbing())
				{
					bool is_selected = false;
					for(PlumbingElement elem : selected)
					{
						if(elem == pair.second)
						{
							is_selected = true;
							break;
						}
					}
					if(is_selected)
					{
						continue;
					}
					nvgStrokeWidth(vg, 2.0f / (float)zoom);
					nvgStrokeColor(vg, nvgRGB(255, 0, 0));
					nvgResetTransform(vg);

					glm::vec2 ppos = pair.second->plumbing.editor_position;

					nvgTranslate(vg, center.x + (ppos.x - cam_center.x) * (float)zoom,
								 center.y + (ppos.y - cam_center.y) * (float)zoom);
					nvgScale(vg, (float)zoom, (float)zoom);

					// Draw the collision envelope, this is expanded
					glm::ivec2 size = pair.second->plumbing.get_editor_size();
					nvgBeginPath(vg);
					nvgRect(vg, -1.0, -1.0, size.x + 2.0, size.y + 2.0);
					nvgStroke(vg);

				}
			}
			i += 3;
		}


	}
}

void PlumbingEditor::draw_junction(NVGcontext *vg, glm::vec2 pos, size_t port_count, bool flip_three) const
{
	float f = 1.0f;
	if(port_count <= 4)
	{
		if (port_count >= 1)
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, pos.x, pos.y);
			nvgLineTo(vg, pos.x, pos.y - f);
			nvgStroke(vg);
			draw_port(vg, pos + glm::vec2(0.0f, -f));
		}

		if (port_count >= 2)
		{
			if(flip_three)
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, pos.x, pos.y);
				nvgLineTo(vg, pos.x + f, pos.y);
				nvgStroke(vg);
				draw_port(vg, pos + glm::vec2(f, 0.0));
			}
			else
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, pos.x, pos.y);
				nvgLineTo(vg, pos.x - f, pos.y);
				nvgStroke(vg);
				draw_port(vg, pos + glm::vec2(-f, 0.0));
			}
		}

		if (port_count >= 3)
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, pos.x, pos.y);
			nvgLineTo(vg, pos.x, pos.y + f);
			nvgStroke(vg);
			draw_port(vg, pos + glm::vec2(0.0f, f));
		}

		if (port_count == 4)
		{
			nvgBeginPath(vg);
			nvgMoveTo(vg, pos.x, pos.y);
			nvgLineTo(vg, pos.x + f, pos.y);
			nvgStroke(vg);
			draw_port(vg, pos + glm::vec2(f, 0.0f));
		}
	}
	else
	{
		// We draw sub junctions to the right, every 2 additional ports one additional subjunction
		size_t put_ports = 0;
		size_t num_subs = ((port_count - 5) / 2) + 2;
		for(size_t i = 0; i < num_subs; i++)
		{
			glm::vec2 rpos = pos + glm::vec2(1.0f, 0.0f) * (float)i;
			if(i == 0)
			{
				draw_junction(vg, rpos, 3);
				put_ports += 3;
			}
			else if(i == num_subs - 1)
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, pos.x + i - 1.0f, pos.y);
				nvgLineTo(vg, pos.x + i, pos.y);
				nvgStroke(vg);
				draw_junction(vg, rpos, port_count - put_ports, true);
				put_ports += port_count - put_ports;
			}
			else
			{
				nvgBeginPath(vg);
				nvgMoveTo(vg, pos.x + i - 1.0f, pos.y);
				nvgLineTo(vg, pos.x + i, pos.y);
				nvgLineTo(vg, pos.x + i, pos.y - f);
				nvgMoveTo(vg, pos.x + i, pos.y);
				nvgLineTo(vg, pos.x + i, pos.y + f);
				nvgStroke(vg);

				size_t put = std::min<size_t>(port_count - put_ports, 2);
				// Top and bottom
				draw_port(vg, rpos + glm::vec2(0.0, -f));
				draw_port(vg, rpos + glm::vec2(0.0, f));
				put_ports += put;
			}
		}
	}
}

void PlumbingEditor::draw_port(NVGcontext *vg, glm::vec2 pos, bool hovered_port) const
{
	if(hovered_port)
	{
		nvgFillColor(vg, nvgRGB(0, 0, 0));
	}
	else
	{
		nvgFillColor(vg, nvgRGB(255, 255, 255));
	}

	nvgBeginPath(vg);
	nvgCircle(vg, pos.x, pos.y, port_radius);
	nvgFill(vg);
	nvgStroke(vg);
}

void PlumbingEditor::handle_vacant_junction()
{
	if(!vacant_junction)
	{
		return;
	}

	PipeJunction* jnc = vacant_junction;
	vacant_junction = nullptr;

	for(size_t i = 0; i < jnc->pipes.size(); i++)
	{
		// TODO: This check may be unnecesary, as is the constant
		if(jnc->pipes[i] == nullptr && jnc->pipes_id[i] == 0xDEADBEEF)
		{
			jnc->pipes.erase(jnc->pipes.begin() + i);
			jnc->pipes_id.erase(jnc->pipes_id.begin() + i);
			break;
		}
	}

	if (jnc->pipes.size() <= 2)
	{
		// We must remove the junction and reconnect the remaining two pipes
		Pipe *p0 = jnc->pipes[0], *p1 = jnc->pipes[1];
		p0->junction = nullptr;
		p0->junction_id = 0;
		p1->junction = nullptr;
		p1->junction_id = 0;

		p0->ma = p1->mb;
		p0->port_a = p1->port_b;
		// Insert a waypoint where the junction was
		p0->waypoints.push_back(jnc->pos);
		// Merge the waypoints
		p0->waypoints.insert(p0->waypoints.begin(),
							 p1->waypoints.begin(), p1->waypoints.end());

		// Remove the [1] pipe and the junction, invalidates pointers but we wont use them again!
		// We must re-create the hovering pipe!
		size_t hovering_pipe_id = hovering_pipe->id;
		veh->plumbing.remove_pipe(p1->id);
		hovering_pipe = veh->plumbing.get_pipe(hovering_pipe_id);
		// Remove the junction
		// TODO: we must be careful as it may be contained in selected or hovered
		veh->plumbing.remove_junction(jnc->id);
	}
}





