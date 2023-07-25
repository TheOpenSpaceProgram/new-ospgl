#include "EditorVehicle.h"
#include <util/InputUtil.h>
#include "EditorScene.h"
#include <physics/glm/BulletGlmCompat.h>
#include <util/fmt/glm.h>
#include <GLFW/glfw3.h>

void EditorVehicle::update_collider(Piece* p)
{
	remove_collider(p);
	// Now create it

	btRigidBody::btRigidBodyConstructionInfo info(0.0, nullptr, p->collider, btVector3(0.0, 0.0, 0.0));
	btRigidBody* rigid_body = new btRigidBody(info);

	btTransform trans = p->get_graphics_transform();
	rigid_body->setWorldTransform(trans);
	rigid_body->setInterpolationWorldTransform(trans);

	RigidBodyUserData* udata = new RigidBodyUserData(p);

	rigid_body->setUserPointer((void*)udata);

	scene->bt_world->addCollisionObject(rigid_body);

	piece_meta[p].collider.rigid = rigid_body;
	piece_meta[p].collider.udata = udata;
}

// Doesn't do anything if not present
void EditorVehicle::remove_collider(Piece* p)
{
	EditorVehiclePiece& mp = piece_meta[p];

	if(mp.collider.rigid != nullptr)
	{
		scene->bt_world->removeCollisionObject(mp.collider.rigid);
		delete mp.collider.rigid;
		delete mp.collider.udata;
		mp.collider.rigid = nullptr;
		mp.collider.udata = nullptr;
	}
}

void EditorVehicle::clear_meta() 
{
	for(auto& mp : piece_meta)
	{
		mp.second.highlight = glm::vec3(0.0f);
		mp.second.draw_out_attachments = false;
		mp.second.draw_in_attachments = true;
		mp.second.attachment_color.clear();
	}
}


void EditorVehicle::deferred_pass(CameraUniforms& cu, bool is_env_map)
{	
	for (Piece* p : veh->all_pieces)
	{
		p->model_node->draw(cu, p->get_graphics_matrix(), drawable_uid, true);
	}
}

void EditorVehicle::forward_pass(CameraUniforms& cu, bool is_env_map)
{
	// We only draw "receiver" models, that is, attachment
	// points which are set to stack (radial goes anywhere)
	for(Piece* p : veh->all_pieces)
	{
		if(draw_attachments)
		{
			for(std::pair<PieceAttachment, bool>& pair : p->attachments)
			{
				draw_attachment(pair, p, cu);
			}
		}

		if(piece_meta[p].highlight != glm::vec3(0.0f, 0.0f, 0.0f))
		{
			draw_highlight(p, piece_meta[p].highlight, cu);
		}
	}

	for(auto& pair : draw_attachment_for)
	{
		draw_attachment(pair.first->attachments[pair.second], pair.first, cu, true);
	}

	draw_attachment_for.clear();
}

void EditorVehicle::shadow_pass(ShadowCamera& cu)
{
	for(Piece* p : veh->all_pieces)
	{
		p->model_node->draw_shadow(cu, p->get_graphics_matrix(), true);
	}
}

void EditorVehicle::update(double dt)
{
	veh->editor_update(dt);

	if(input->key_down(GLFW_KEY_S))
	{
		auto table = cpptoml::make_table();
		GenericSerializer<EditorVehicle>::serialize(*this, *table);
		SerializeUtil::write_to_file(*table, "udata/vehicles/debug.toml");
	}
}


void EditorVehicle::init(sol::state* lua_state)
{
	veh = new Vehicle();
	auto file = SerializeUtil::load_file("udata/vehicles/debug.toml");
	auto loader = VehicleLoader(*file, *veh, true);

	scene->part_id = loader.vpart_id;
	scene->piece_id = loader.vpiece_id;

	// Load the different models
	std::string model_path = *osp->assets->load_toml("core:meshes/editor_attachment.toml")
			->get_as<std::string>("model");

	AssetHandle<Model> model = AssetHandle<Model>(model_path);

	stack_model = GPUModelNodePointer(model.duplicate(), "stack");
	radial_model = GPUModelNodePointer(model.duplicate(), "radial");
	stack_radial_model = GPUModelNodePointer(model.duplicate(), "stack_radial");
	receive_model = GPUModelNodePointer(model.duplicate(), "receive");

	mat_hover = AssetHandle<Material>("core:mat_hover.toml");

	draw_attachments = false;

	veh->init(lua_state);

	// Load all colliders
	for(Piece* p : veh->all_pieces)
	{
		piece_meta[p] = EditorVehiclePiece();
		update_collider(p);
	}
}


EditorVehicle::EditorVehicle(EditorScene* sc) 
	: Drawable(), scene(sc)
{
}

void EditorVehicle::draw_highlight(Piece* p, glm::vec3 color, CameraUniforms& cu)
{
	// Override the hover color
	MaterialOverride over = MaterialOverride();
	over.uniforms["color"] = Uniform(color);

	p->model_node->draw_override(cu, &(*mat_hover), p->get_graphics_matrix(), drawable_uid, &over, true);
}

void EditorVehicle::draw_attachment(std::pair<PieceAttachment, bool>& pair, Piece* p, CameraUniforms& cu,
									bool is_particular)
{
	PieceAttachment& attch = pair.first;
	if(attch.hidden){ return; }

	glm::dvec3 pos = p->get_marker_position(attch.marker);
	glm::dquat quat = p->get_marker_rotation(attch.marker);

	glm::dmat4 model = glm::translate(glm::dmat4(1.0), pos);
	model = glm::scale(model, glm::dvec3(attch.size * 0.15));
	model = model * glm::toMat4(quat);

	model = p->get_graphics_matrix() * model;

	glm::vec4 color = glm::vec4(1.0f);
	auto it = piece_meta[p].attachment_color.find(pair.first.marker);
	if(it != piece_meta[p].attachment_color.end())
	{
		color = it->second;
	}

	MaterialOverride mat_over;
	mat_over.uniforms["color"] = glm::vec3(color);
	mat_over.uniforms["transparency"] = 1.0f - color.a;

	if(!pair.second && piece_meta[p].draw_in_attachments && attch.stack == true && !is_particular)
	{
		receive_model->draw_override(cu, nullptr, model, drawable_uid, &mat_over, true);
	}

	if((!pair.second && piece_meta[p].draw_out_attachments) || is_particular)
	{
		if(attch.stack && !attch.radial)
		{
			stack_model->draw_override(cu, nullptr, model, drawable_uid, &mat_over, true);
		}
		else if(attch.radial && !attch.stack)
		{
			radial_model->draw_override(cu, nullptr, model, drawable_uid, &mat_over, true);
		}
		else if(attch.radial && attch.stack)
		{
			stack_radial_model->draw_override(cu, nullptr, model, drawable_uid, &mat_over, true);
		}
	}

}

void EditorVehicle::update_collider_hierarchy(Piece *p)
{
	std::vector<Piece*> children = veh->get_children_of(p);
	update_collider(p);
	for(Piece* child : children)
	{
		update_collider(child);
	}
}

void EditorVehicle::attach(Piece *piece, Piece *to, const std::string &attachment_id,
						   const std::string &target_attachment_id)
{
	piece->attached_to = to;
	piece->from_attachment = attachment_id;
	piece->to_attachment = target_attachment_id;

	auto clones = veh->meta.find_symmetry_instances(to, true);

	// Attach to first and find relative transform
	glm::dmat4 original_relative = glm::inverse(to->get_graphics_matrix()) * piece->get_graphics_matrix();

	struct PiecesToMode
	{
		// Offset in any of the roots
		int to_idx;
		std::vector<Piece*> from;
		// Root for each of the "from" pieces
		std::vector<int> sub_root;
	};

	std::unordered_map<SymmetryMode*, std::vector<PiecesToMode>> ptm;

	// Make clones for all subsymmetries
	for(size_t i = 0; i < clones.size(); i++)
	{
		Piece* new_root;
		if(clones[i].p == to)
		{
			new_root = piece;
		}
		else
		{
			new_root = veh->duplicate(piece, &scene->lua_state,
									  &scene->piece_id, &scene->part_id);
		}

		new_root->attached_to = to;

		// Add pieces to all symmetry groups clones[i] belongs to (to guarantee further duplication)
		for(SymmetryMode* g : clones[i].modes)
		{
			auto idx = g->get_piece_sub_index(clones[i].p);

			auto it = ptm.find(g);
			if(it == ptm.end())
			{
				ptm[g] = std::vector<PiecesToMode>();
				it = ptm.find(g);
			}

			bool found = false;
			for(auto& p : it->second)
			{
				if(p.to_idx == idx.second)
				{
					// This is where we want to be
					p.from.push_back(new_root);
					p.sub_root.push_back(idx.first);
					found = true;
					break;
				}
			}

			if(!found)
			{
				PiecesToMode n;
				n.to_idx = idx.second;
				n.from.push_back(new_root);
				n.sub_root.push_back(idx.first);
				it->second.push_back(n);
			}
		}

		// The user-positioned piece is already correct!
		if(new_root != piece)
		{
			new_root->attached_to = clones[i].p;
			// Position the root piece (children follow automatically)
			glm::dmat4 tform_new = clones[i].p->get_graphics_matrix() * original_relative;
			veh->move_piece_mat(new_root, tform_new);
			update_collider_hierarchy(new_root);
		}

	}

	// Add new pieces to symmetry modes, this also serves as a "safety check"
	for(const auto& pair : ptm)
	{
		SymmetryMode* g = pair.first;

		for(const PiecesToMode& p : pair.second)
		{
			logger->check(p.from.size() == pair.first->clones.size(), "Mismatched number of new elements for symmetry");

			int offset = 1;

			// Sort p.from in the same order as roots
			std::vector<Piece*> cp; cp.resize(p.from.size());
			for(int i = 0; i < p.from.size(); i++)
			{
				auto idx = pair.first->get_piece_sub_index(p.from[i]->attached_to);
				cp[idx.first] = p.from[i];
			}

			for(int i = 0; i < cp.size(); i++)
			{
				std::vector<Piece *> new_child = veh->get_children_of(cp[i]);
				g->all_in_symmetry.insert(g->all_in_symmetry.begin() + (i * g->clone_depth + p.to_idx + offset), cp[i]);
				offset++;
				g->all_in_symmetry.insert(g->all_in_symmetry.begin() + (i * g->clone_depth + p.to_idx + offset),
									   new_child.begin(), new_child.end());
				offset += new_child.size();
			}

			g->update_clone_depth();
		}

	}

}

void EditorVehicle::detach(Piece *piece)
{
	// TODO: This may be surprisingly expensive with many nested symmetry groups!

	// If children of our clones fully contain all roots of a symmetry group,
	// move them so that the symmetry group is not lost
	// (when this is not possible, sadly the groups are lost!)
	auto clones = veh->meta.find_symmetry_instances(piece, true);
	std::vector<SymmetryMode*> to_move;
	// indices in piece_children.push_back(piece) for each clone!
	std::vector<std::vector<size_t>> new_root_indices;

	// Many nested loops, but they should exit fairly quickly most of the time!
	for(const auto& clone : clones)
	{
		if(clone.p == piece)
			continue;

		std::vector<Piece*> all_p = veh->get_children_of(clone.p);
		all_p.push_back(clone.p);

		for(SymmetryMode* m : veh->meta.symmetry_groups)
		{
			bool all_found = true;
			std::vector<size_t> new_indices;
			for(size_t j = 0; j < m->clones.size(); j++)
			{
				bool found = false;
				for(size_t k = 0; k < all_p.size(); k++)
				{
					if(all_p[k] == m->clones[j])
					{
						new_indices.push_back(k);
						found = true;
						break;
					}
				}

				if(!found)
				{
					all_found = false;
					break;
				}
			}

			if(all_found)
			{
				to_move.push_back(m);
				new_root_indices.push_back(new_indices);
			}
		}
	}

	std::vector<Piece*> all_p = veh->get_children_of(piece);
	all_p.push_back(piece);

	for(size_t i = 0; i < to_move.size(); i++)
	{
		std::vector<Piece*> vec;
		for(size_t idx : new_root_indices[i])
		{
			vec.push_back(all_p[idx]);
		}
		to_move[i]->new_root_clones(this, vec);
	}

	// Step 2: Dettach the piece, destroy all clones and remove them from the
	// symmetry groups they belong to
	piece->attached_to = nullptr;
	std::set<SymmetryMode*> all_modes;
	for(const auto& clone : clones)
	{
		for(SymmetryMode* m : clone.modes)
		{
			all_modes.insert(m);
			m->remove_piece_and_children_from_symmetry(this, clone.p);
		}

		if(clone.p != piece)
		{
			// Actual removal of the pieces
			std::vector<Piece *> child = veh->get_children_of(clone.p);
			clone.p->attached_to = nullptr;
			remove_collider(clone.p);
			piece_meta.erase(clone.p);
			veh->remove_piece(clone.p);
			delete clone.p;
			for (Piece *p: child)
			{
				remove_collider(p);
				piece_meta.erase(p);
				veh->remove_piece(p);
				delete p;
			}
		}
	}

	for(SymmetryMode* m : all_modes)
	{
		m->cleanup();
		m->update_clone_depth();
	}

	veh->meta.cleanup_symmetry_groups();

}


void GenericSerializer<EditorVehicle>::serialize(const EditorVehicle& what, cpptoml::table& target)
{
	// Code is very different for in flight vehicle serialization as the editor creates a big
	// bunch of meta-data, but the basic stuff provided by the vehicle serialization works
	// fine as a "starting material"
	GenericSerializer<Vehicle>::serialize(*what.veh, target);
	
	// We not attach all the metadata
}

void GenericSerializer<EditorVehicle>::deserialize(EditorVehicle& to, const cpptoml::table& from) 
{
	
}