#include "BuildingPrototype.h"
#include <assets/AssetManager.h>

BuildingPrototype* load_building_prototype(ASSET_INFO, const cpptoml::table & cfg)
{
	std::shared_ptr<cpptoml::table> toml = SerializeUtil::load_file(path);

	BuildingPrototype* out = new BuildingPrototype(toml, ASSET_INFO_P);

	return out;
}

BuildingPrototype::BuildingPrototype(std::shared_ptr<cpptoml::table> table, ASSET_INFO) : Asset(ASSET_INFO_P)
{
	std::string model_path = *table->get_as<std::string>("model");

	model = GPUModelPointer(AssetHandle<Model>(model_path));

	auto it = model->node_by_name.find("building");
	logger->check(it != model->node_by_name.end(),
			   "Could not find building node on model while loading '{}'", get_asset_id());
	Node* building = it->second;

	collider = nullptr;
	for (Node* child : building->children)
	{
		if (child->name.rfind(Model::MARK_PREFIX, 0) == 0)
		{
			// Load a mark
		}
		else if (child->name.rfind(Model::COLLIDER_PREFIX, 0) == 0)
		{
			if (collider != nullptr)
			{
				logger->fatal("Buildings can only have one collider!");
			}

			ModelColliderExtractor::load_collider(&collider, child);
			collider->setMargin(0.005);
		}
	}


}
