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


void EditorVehicle::deferred_pass(CameraUniforms& cu)
{	
	for (Piece* p : veh->all_pieces)
	{
		p->model_node->draw(cu, p->get_graphics_matrix(), drawable_uid, true);
	}
}

void EditorVehicle::forward_pass(CameraUniforms& cu)
{
	// We only draw "receiver" models, that is, attachment
	// points which are set to stack (radial goes anywhere)
	for(Piece* p : veh->all_pieces)
	{
		if(draw_attachments)
		{
			for(std::pair<PieceAttachment, bool>& pair : p->attachments)
			{
				PieceAttachment& attch = pair.first;
				if(attch.hidden){ continue; }

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

				if(!pair.second && piece_meta[p].draw_in_attachments && attch.stack == true)
				{
					receive_model->draw_override(cu, nullptr, model, drawable_uid, &mat_over, true);
				}
				
				if(!pair.second && piece_meta[p].draw_out_attachments)
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
		}

		if(piece_meta[p].highlight != glm::vec3(0.0f, 0.0f, 0.0f))
		{
			draw_highlight(p, piece_meta[p].highlight, cu);
		}
	}
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
	veh = new Vehicle();
	SerializeUtil::read_file_to("udata/vehicles/debug.toml", *veh);
	
	// Load the different models
	std::string model_path = *SerializeUtil::load_file(assets->resolve_path("core:meshes/editor_attachment.toml"))
		->get_as<std::string>("model");

	AssetHandle<Model> model = AssetHandle<Model>(model_path);

	stack_model = GPUModelNodePointer(model.duplicate(), "stack");
	radial_model = GPUModelNodePointer(model.duplicate(), "radial");	
	stack_radial_model = GPUModelNodePointer(model.duplicate(), "stack_radial");
	receive_model = GPUModelNodePointer(model.duplicate(), "receive");

	mat_hover = AssetHandle<Material>("core:mat_hover.toml");

	draw_attachments = false;

}

void EditorVehicle::draw_highlight(Piece* p, glm::vec3 color, CameraUniforms& cu)
{
	// Override the hover color
	MaterialOverride over = MaterialOverride();
	over.uniforms["color"] = Uniform(color);

	p->model_node->draw_override(cu, &(*mat_hover), p->get_graphics_matrix(), drawable_uid, &over, true);
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