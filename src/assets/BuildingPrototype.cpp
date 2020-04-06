#include "BuildingPrototype.h"
#include <assets/AssetManager.h>

BuildingPrototype* load_building_prototype(const std::string & path, const std::string & name, const std::string & pkg, const cpptoml::table & cfg)
{
	std::shared_ptr<cpptoml::table> toml = SerializeUtil::load_file(path);

	BuildingPrototype* out = new BuildingPrototype(toml);

	return out;
}

BuildingPrototype::BuildingPrototype(std::shared_ptr<cpptoml::table> table)
{
	std::string model_path = *table->get_as<std::string>("model");

	model = GPUModelPointer(AssetHandle<Model>(model_path));

	Node* building = model->node_by_name["building"];
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
