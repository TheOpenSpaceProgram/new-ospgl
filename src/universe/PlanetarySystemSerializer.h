#pragma once

template<>
class GenericSerializer<PlanetarySystem>
{
public:

	static int levels_of_parent(const SystemElement& body)
	{
		int base = body.is_primary ? 1 : 0;
		if (body.parent == nullptr)
		{
			return base;
		}
		else
		{
			if (body.parent->parent == &body)
			{
				logger->fatal("Circular dependencies are not allowed!");
			}

			return levels_of_parent(*body.parent) + base + 1;
		}
	}

	static void serialize(const PlanetarySystem& what, cpptoml::table& target)
	{

	}

	static void deserialize(PlanetarySystem& to, const cpptoml::table& from)
	{
		to.elements = std::vector<SystemElement>();
		
		// We first obtain all bodies and parent relations
		std::unordered_map<std::string, std::string> parents;
		std::vector<SystemElement> temp_array;

		auto planet_list = from.get_table_array("element");
		TOML_CHECK_FUNC(planet_list != nullptr, "Element List could not be obtained");

		for (const auto& planet : *planet_list)
		{
			temp_array.push_back(SystemElement());
			to.elements.push_back(SystemElement());
			::deserialize(to.elements[to.elements.size() - 1], *planet);
			temp_array[temp_array.size() - 1] = to.elements[to.elements.size() - 1];

			std::string name = to.elements[to.elements.size() - 1].name;

			parents[name] = planet->get_qualified_as<std::string>("parent").value_or("");
		}

		// Assign parents
		for (size_t i = 0; i < temp_array.size(); i++)
		{
			temp_array[i].parent = nullptr;
			to.elements[i].parent = nullptr;

			if (parents[temp_array[i].name] != "")
			{
				for (size_t j = 0; j < temp_array.size(); j++)
				{
					if (temp_array[j].name == parents[temp_array[i].name])
					{
						TOML_CHECK_FUNC(j != i, "Can't set a body's parent to itself");
						temp_array[i].parent = &to.elements[j];
						to.elements[i].parent = &to.elements[j];
					}
				}

				TOML_CHECK_FUNC(temp_array[i].parent != nullptr, "A body's parent was invalid");
			}
		}

		// Sort by parent order so position computation requires no second loop
		std::sort(temp_array.begin(), temp_array.end(), [](SystemElement& a, SystemElement& b)
		{
			return levels_of_parent(a) < levels_of_parent(b);
		});

		// Copy array
		to.elements = temp_array;

		SystemElement star_element;
		star_element.type = SystemElement::STAR;

		// Insert star
		to.elements.insert(to.elements.begin(), star_element);

		// Reassign parents and indexes
		for (size_t i = 0; i < to.elements.size(); i++)
		{
			to.elements[i].index = i;
			to.elements[i].parent = nullptr;

			if (i != 0)
			{
				if (parents[to.elements[i].name] != "")
				{
					for (size_t j = 0; j < to.elements.size(); j++)
					{
						if (to.elements[j].name == parents[to.elements[i].name])
						{
							TOML_CHECK_FUNC(j != i, "Can't set a body's parent to itself");
							to.elements[i].parent = &to.elements[j];
						}
					}
				}


				if (to.elements[i].parent == nullptr)
				{
					to.elements[i].parent = &to.elements[0];
				}
			}
		}

		// Set barycenter's masses
		// We traverse backwards to guarantee proper ordering in case
		// a barycenter is the parent of another
		for (int i = (int)to.elements.size() - 1; i >= 0; i--)
		{
			if (to.elements[i].type == SystemElement::BARYCENTER)
			{
				for (size_t j = 0; j < to.elements.size(); j++)
				{
					if (to.elements[j].parent == &to.elements[i])
					{
						if (to.elements[j].type == SystemElement::BARYCENTER)
						{
							// TODO: Is this easy to implement?
							logger->fatal("Nested barycenters are not allowed!");
						}
						else
						{
							if (to.elements[j].is_primary)
							{
								to.elements[i].as_barycenter->primary = &to.elements[j];
							}
							else
							{
								to.elements[i].as_barycenter->secondary = &to.elements[j];
							}
						}
					}
				}

				// Compute barycenter primaries radius
				// (No orbit is neccesary as a barycenter primary is 
				// always opposite of its secondary)
				// TODO: Add this to the docs, variable smajor axis is not 
				// updated in barycenters
				auto elems = to.elements[i].as_barycenter->secondary->orbit.to_orbit_at(0.0);
				double smajor = elems.smajor_axis;

				double mc = to.elements[i].as_barycenter->primary->get_mass() / to.elements[i].as_barycenter->secondary->get_mass();
				to.elements[i].as_barycenter->primary->barycenter_radius = smajor / (1.0 + mc);
				to.elements[i].as_barycenter->primary->orbit = to.elements[i].as_barycenter->secondary->orbit;

				// Used only for representation
				if (to.elements[i].as_barycenter->primary->orbit.is_nasa_data)
				{
					to.elements[i].as_barycenter->primary->orbit.data.nasa_data.smajor_axis = to.elements[i].as_barycenter->primary->barycenter_radius;
				}
				else
				{
					to.elements[i].as_barycenter->primary->orbit.data.normal_data.smajor_axis = to.elements[i].as_barycenter->primary->barycenter_radius;
				}
				
			}
		}

	

		// Star
		to.elements[0].as_star = new Star();
		::deserialize(*to.elements[0].as_star, *from.get_table_qualified("star"));
		to.elements[0].name = from.get_qualified_as<std::string>("star.name").value_or("Star");
	}
};