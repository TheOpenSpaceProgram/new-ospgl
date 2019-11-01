#pragma once

template<>
class GenericSerializer<PlanetarySystem>
{
public:

	static int levels_of_parent(const PlanetaryBody& body)
	{
		if (body.parent == nullptr)
		{
			return 0;
		}
		else
		{
			if (body.parent->parent == &body)
			{
				logger->fatal("Circular dependencies are not allowed!");
			}

			return levels_of_parent(*body.parent) + 1;
		}
	}

	static void serialize(const PlanetarySystem& what, cpptoml::table& target)
	{
		for (size_t i = 0; i < what.bodies.size(); i++)
		{

		}
	}

	static void deserialize(PlanetarySystem& to, const cpptoml::table& from)
	{

		// We first obtain all bodies and parent relations
		std::unordered_map<std::string, std::string> parents;
		std::vector<PlanetaryBody> temp_array;

		auto planet_list = from.get_table_array("planet");
		TOML_CHECK_FUNC(planet_list != nullptr, "Planet List could not be obtained");

		for (const auto& planet : *planet_list)
		{
			temp_array.push_back(PlanetaryBody());
			to.bodies.push_back(PlanetaryBody());
			::deserialize(to.bodies[to.bodies.size() - 1], *planet);
			temp_array[temp_array.size() - 1] = to.bodies[to.bodies.size() - 1];

			std::string name = to.bodies[to.bodies.size() - 1].name;

			parents[name] = planet->get_qualified_as<std::string>("parent").value_or("");
		}

		// Assign parents
		for (size_t i = 0; i < temp_array.size(); i++)
		{
			temp_array[i].parent = nullptr;
			to.bodies[i].parent = nullptr;

			if (parents[temp_array[i].name] != "")
			{
				for (size_t j = 0; j < temp_array.size(); j++)
				{
					if (temp_array[j].name == parents[temp_array[i].name])
					{
						TOML_CHECK_FUNC(j != i, "Can't set a body's parent to itself");
						temp_array[i].parent = &to.bodies[j];
						to.bodies[i].parent = &to.bodies[j];
					}
				}

				TOML_CHECK_FUNC(temp_array[i].parent != nullptr, "A body's parent was invalid");
			}
		}

		// Sort by parent order so position computation requires no second loop
		std::sort(temp_array.begin(), temp_array.end(), [](PlanetaryBody& a, PlanetaryBody& b)
		{
			return levels_of_parent(a) < levels_of_parent(b);
		});

		// Copy array
		to.bodies = temp_array;

		// Reassign parents and indexes
		for (size_t i = 0; i < to.bodies.size(); i++)
		{
			to.bodies[i].index = i;
			to.bodies[i].parent = nullptr;

			if (parents[to.bodies[i].name] != "")
			{
				for (size_t j = 0; j < to.bodies.size(); j++)
				{
					if (to.bodies[j].name == parents[to.bodies[i].name])
					{
						TOML_CHECK_FUNC(j != i, "Can't set a body's parent to itself");
						to.bodies[i].parent = &to.bodies[j];
					}
				}

				TOML_CHECK_FUNC(to.bodies[i].parent != nullptr, "A body's parent was invalid");
			}
		}

		// Star
		SAFE_TOML_GET(to.star_mass, "star.mass", double);


	}
};