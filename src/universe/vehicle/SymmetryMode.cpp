#include "SymmetryMode.h"
#include <game/scenes/editor/interfaces/ModifyInterface.h>
#include <game/scenes/editor/gui/ModifyPanel.h>
#include <game/scenes/editor/EditorVehicleInterface.h>
#include <game/scenes/editor/EditorScene.h>


void SymmetryMode::take_gui_control(ModifyPanel* panel, ModifyInterface* mod_int, EditorVehicleInterface* edveh_int)
{
	env["symmetry_panel"] = panel;
	env["modify_interface"] = mod_int;
	env["edveh_interface"] = edveh_int;
	LuaUtil::call_function_if_present(env["take_gui_control"]);
}


void SymmetryMode::gui_go_back()
{
	LuaUtil::call_function_if_present(env["gui_go_back"]);
}

void SymmetryMode::leave_gui_control()
{
	LuaUtil::call_function_if_present(env["leave_gui_control"]);
	env["symmetry_panel"] = nullptr;
	env["modify_interface"] = nullptr;
}

void SymmetryMode::init(sol::state *in_state, EditorVehicle* in_vehicle, const std::string &in_pkg)
{
	this->sc = in_vehicle->scene;

	auto[pkg, name] = osp->assets->get_package_and_name(script_path, in_pkg);

	env = sol::environment(*in_state, sol::create, in_state->globals());
	lua_core->load((sol::table&)env, pkg);
	env["symmetry_mode"] = this;
	env["vehicle"] = in_vehicle->veh;
	env["editor_vehicle"] = in_vehicle;
	env["osp"] = osp;

	std::string full_path = osp->assets->res_path + pkg + "/" + name;
	auto result = (*in_state).safe_script_file(full_path, env);
	if(!result.valid())
	{
		sol::error err = result;
		logger->fatal("Lua Error loading symmetry mode:\n{}", err.what());
	}
}

std::vector<Piece*> SymmetryMode::make_clones(EditorVehicle* edveh, int count)
{
	Vehicle* veh = root->in_vehicle;

	remove_non_root(edveh);
	std::vector<Piece*> out;
	std::vector<Piece*> all_new_pieces;

	out.push_back(root);
	for(int i = 1; i < count; i++)
	{
		Piece* new_root = veh->duplicate(root, &sc->lua_state, &sc->piece_id, &sc->part_id);
		out.push_back(new_root);
		all_new_pieces.push_back(new_root);
		std::vector<Piece*> new_child = veh->get_children_of(new_root);
		all_new_pieces.insert(all_new_pieces.end(), new_child.begin(), new_child.end());
		clone_depth = new_child.size() + 1;
	}
	clones = out;

	// update all_in_symmetry array
	auto root_children = veh->get_children_of(root);
	all_in_symmetry.clear();
	all_in_symmetry.push_back(root);
	all_in_symmetry.insert(all_in_symmetry.end(), root_children.begin(), root_children.end());
	all_in_symmetry.insert(all_in_symmetry.end(), all_new_pieces.begin(), all_new_pieces.end());

	return out;
}

bool SymmetryMode::is_piece_in_symmetry(Piece *p)
{
	return vector_contains(all_in_symmetry, p);
}

void SymmetryMode::remove(EditorVehicle* edveh)
{
	if(root == nullptr)
		return;

	remove_non_root(edveh);

	// Return root to original position
	root->detach();
	root->in_vehicle->move_piece(root, original_root_pos, original_root_rot, "");
	root->in_vehicle->update_attachments();
	edveh->update_collider_hierarchy(root);
}

void SymmetryMode::remove_non_root(EditorVehicle* edveh)
{
	Vehicle* veh = root->in_vehicle;
	// Remove all old clones, except first one (root)
	for(int i = 1; i < clones.size(); i++)
	{
		remove_piece_and_children(edveh, clones[i]);
	}

}

Piece* SymmetryMode::find_father_clone(Piece *p)
{
	Piece* ptr = p;
	while(ptr)
	{
		if(vector_contains(clones, ptr))
		{
			return ptr;
		}
		ptr = ptr->attached_to;
	}

	return nullptr;
}

void SymmetryMode::remove_all_but(EditorVehicle* edveh, Piece* exp)
{
	Vehicle* veh = root->in_vehicle;
	auto children = veh->get_children_of(exp);

	for(Piece* p : all_in_symmetry)
	{
		if(!vector_contains(children, p) && p != exp)
		{
			veh->remove_piece(p);
			edveh->remove_collider(p);
			edveh->piece_meta.erase(p);
			delete p;
		}
	}

	all_in_symmetry.clear();

}

bool SymmetryMode::disconnect(EditorVehicle* edveh, Piece *piece)
{
	// If we pick up a root piece, remove it and the symmetry group
	if(vector_contains(clones, piece))
	{
		remove_all_but(edveh, piece);
		return true;
	}
	else
	{
		// Remove the single piece and update
		auto index = get_piece_sub_index(piece);
		for(int i = 0; i < clones.size(); i++)
		{
			if(i != index.first)
			{
				Piece* sub_root = all_in_symmetry[i * clone_depth + index.second];
				remove_piece_and_children(edveh, sub_root);
			}
		}
		remove_piece_and_children_from_symmetry(edveh, piece);
		cleanup();
		update_clone_depth();

		return false;
	}
}

std::pair<int, int> SymmetryMode::get_piece_sub_index(Piece *p)
{
	Piece* subroot = find_father_clone(p);
	if(subroot == nullptr)
	{
		return std::make_pair(-1, -1);
	}
	int clone = -1;
	for(clone = 0; clone < clones.size(); clone++)
	{
		if(subroot == clones[clone])
			break;
	}
	logger->check(clone != -1, "Symmetry group malformed");
	// Travel down from subroot
	int i = 0;
	bool found = false;
	for(i = 0; i < clone_depth; i++)
	{
		if(all_in_symmetry[clone * clone_depth + i] == p)
		{
			found = true;
			break;
		}
	}

	logger->check(found, "Symmetry group malformed");

	return std::make_pair(clone, i);
}


void SymmetryMode::remove_piece(EditorVehicle* edveh, Piece *p)
{
	Vehicle* veh = p->in_vehicle;
	veh->remove_piece(p);
	edveh->remove_collider(p);
	edveh->piece_meta.erase(p);
	remove_piece_from_symmetry(edveh, p);
	delete p;

}

void SymmetryMode::remove_piece_and_children(EditorVehicle* edveh, Piece *p)
{
	std::vector<Piece*> child = p->in_vehicle->get_children_of(p);
	for(Piece* sp : child)
	{
		remove_piece(edveh, sp);
	}
	remove_piece(edveh, p);
}

void SymmetryMode::cleanup()
{
	for(auto it = all_in_symmetry.begin(); it != all_in_symmetry.end(); )
	{
		if(*it == nullptr)
			it = all_in_symmetry.erase(it);
		else
			it++;
	}
}

void SymmetryMode::update_clone_depth()
{
	// This very simple algorithm counts how long does it take to reach clones[1]
	if(clones.size() == 1)
	{
		clone_depth = all_in_symmetry.size();
		return;
	}

	for(int i = 0; i < all_in_symmetry.size(); i++)
	{
		if(all_in_symmetry[i] == clones[1])
		{
			clone_depth = i;
			return;
		}
	}
	logger->check(all_in_symmetry.size() == 0, "Symmetry malformed");
}

void SymmetryMode::remove_piece_and_children_from_symmetry(EditorVehicle *edveh, Piece *p)
{
	std::vector<Piece*> child = p->in_vehicle->get_children_of(p);
	for(Piece* sp : child)
	{
		remove_piece_from_symmetry(edveh, sp);
	}
	remove_piece_from_symmetry(edveh, p);

}

void SymmetryMode::remove_piece_from_symmetry(EditorVehicle *edveh, Piece *p)
{
	// TODO: This could be optimized thanks to how the array is structured
	for(auto & i : all_in_symmetry)
	{
		if(i == p)
		{
			i = nullptr;
			return;
		}
	}

}

std::vector<SymmetryInstance> SymmetryMode::find_clones(Piece *p, bool include_p)
{
	std::vector<SymmetryInstance> out;

	auto idx = get_piece_sub_index(p);
	if(idx.first == -1)
	{
		// It was not in array
		return out;
	}

	for(size_t i = 0; i < clones.size(); i++)
	{
		if(i != idx.first || include_p)
		{
			SymmetryInstance clone;
			clone.p = all_in_symmetry[i * clone_depth + idx.second];
			clone.modes.push_back(this);
			out.push_back(clone);
		}
	}

	return out;
}

void SymmetryMode::show_symmetry(EditorVehicle* veh)
{
	veh->clear_meta();

	// Highlight member parts in blue
	for(Piece* p : all_in_symmetry)
	{
		veh->piece_meta[p].highlight = glm::vec3(0.0f, 0.0f, 1.0f);
	}

	// Highlight cloned roots in green
	for(Piece* p : clones)
	{
		veh->piece_meta[p].highlight = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	// Highlight final root in red
	veh->piece_meta[root].highlight = glm::vec3(1.0f, 0.0f, 0.0f);

}

void SymmetryMode::new_root_clones(EditorVehicle* edveh, std::vector<Piece*> p)
{
	logger->check(p.size() == clones.size(), "Invalid number of clones given");

	// quick safety check
	int old_size = all_in_symmetry.size();
	all_in_symmetry.clear();

	// We now build the all_pieces_in_symmetry
	for(Piece* new_root : p)
	{
		// TODO: This assumes good ordering of get_children_of?
		// May break on some VERY WEIRD cases (manual vehicle modification?)
		std::vector<Piece*> children = edveh->veh->get_children_of(new_root);
		all_in_symmetry.push_back(new_root);
		all_in_symmetry.insert(all_in_symmetry.end(), children.begin(), children.end());
	}

	clones = p;
	root = clones[0];
	logger->check(all_in_symmetry.size() == old_size, "Malformed new root pieces");
}


