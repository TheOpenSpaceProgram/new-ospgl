#include "LuaModel.h"
#include <assets/Model.h>

void LuaModel::load_to(sol::table &table)
{
	table.new_usertype<Node>("node", sol::no_constructor,
		  "draw", &Node::draw,
		  "draw_shadow", &Node::draw_shadow,
		  "draw_override", &Node::draw_override,
		  "children", &Node::children,
		  "get_children_recursive", &Node::get_children_recursive,
		  "extract_collider", [](Node* base, Node* n)
		  {
			btCollisionShape *shape;
			ModelColliderExtractor::load_collider(&shape, n);

			glm::dmat4 tform = base->get_tform(n);
			// For lua gargabe collection
			auto shared_ptr = std::shared_ptr<btCollisionShape>(shape);
			return std::make_pair(shared_ptr, tform);
		   },
	      "get_tform_of", &Node::get_tform,
		  "get_bounds_of", &Node::get_bounds,
		  "get_child_deep", &Node::get_child,
		  "name", &Node::name);

	table.new_usertype<MaterialOverride>("mat_override", sol::no_constructor,
		 "from_table", [](const sol::table& table)
		 {
			MaterialOverride out;

			return out;
		 });

	table.new_usertype<Model>("model", sol::no_constructor,
		  "get_gpu", [](Model* model)
		  {
			LuaGPUModelPointer ptr(LuaAssetHandle<Model>(AssetHandle<Model>(model->get_asset_id())));
			model->get_gpu();
			return std::move(ptr);
		  },
		  "get_node", sol::overload([](Model* model, const std::string& name)
		  {
			  auto it = model->node_by_name.find(name);
			  if(it != model->node_by_name.end())
				  return std::optional<Node*>(it->second);
			  else
				  return std::optional<Node*>();
		  },
		  [](Model* model)
		  {
			  return model->root;
		  }));

	table.new_usertype<LuaGPUModelPointer>("gpu_pointer", sol::no_constructor,
		   "move", &LuaGPUModelPointer::move,
		   "get_node", sol::overload(&LuaGPUModelPointer::get_node,
									 [](LuaGPUModelPointer* ptr){ return ptr->get_root(); }));
}

Node* LuaGPUModelPointer::get_node(const std::string &name)
{
	auto it = model_handle->node_by_name.find(name);
	logger->check(it != model_handle->node_by_name.end(), "Could not find node named {}", name);
	return it->second;
}

Node* LuaGPUModelPointer::get_root()
{
	return model_handle->root;
}

LuaGPUModelPointer LuaGPUModelPointer::move()
{
	return LuaGPUModelPointer(std::move(*this));
}

LuaGPUModelPointer::~LuaGPUModelPointer()
{
	if(model_handle.data != nullptr)
	{
		model_handle->free_gpu();
	}
}
