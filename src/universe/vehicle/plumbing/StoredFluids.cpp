#include "StoredFluids.h"

StoredFluids StoredFluids::modify(const StoredFluids &b)
{
	StoredFluids out;

	for(const auto& pair : b.contents)
	{
		auto it = contents.find(pair.first);
		if(it == contents.end())
		{
			// We create a new AssetHandle here to store it as we are creating the key
			contents[pair.first.duplicate()] = pair.second;
			if(pair.second.gas_mass < 0.0f)
			{
				contents.at(pair.first).gas_mass = 0.0f;
			}
			if(pair.second.liquid_mass < 0.0f)
			{
				contents.at(pair.first).liquid_mass = 0.0f;
			}
		}
		else
		{
			out.contents[pair.first.duplicate()] = StoredFluid();

			if(pair.second.gas_mass < 0.0f)
			{
				float original = it->second.gas_mass;
				it->second.gas_mass -= pair.second.gas_mass;
				if(it->second.gas_mass < 0.0f)
				{
					out.contents.at(it->first).gas_mass = original;
				}
				else
				{
					out.contents.at(it->first).gas_mass = pair.second.gas_mass;
				}
			}
			else
			{
				it->second.gas_mass += pair.second.gas_mass;
			}

			if(pair.second.liquid_mass < 0.0f)
			{
				float original = it->second.liquid_mass;
				it->second.liquid_mass -= pair.second.liquid_mass;
				if(it->second.liquid_mass < 0.0f)
				{
					out.contents.at(it->first).liquid_mass = original;
				}
				else
				{
					out.contents.at(it->first).liquid_mass = pair.second.liquid_mass;
				}
			}
			else
			{
				it->second.liquid_mass += pair.second.liquid_mass;
			}
		}
	}

	return out;
}

StoredFluids StoredFluids::multiply(float value)
{
	StoredFluids out;

	for(const auto& pair : contents)
	{
		out.contents[pair.first.duplicate()] = pair.second;
		out.contents.at(pair.first).gas_mass *= value;
		out.contents.at(pair.first).liquid_mass *= value;
	}

	return out;
}

void StoredFluids::generate_internal_contents()
{

}

void StoredFluids::remove_empty_fluids()
{

	generate_internal_contents();
}

void StoredFluids::add_fluid(const AssetHandle<PhysicalMaterial>& mat, float liquid_mass, float gas_mass)
{
	StoredFluids tmp;
	tmp.contents[mat.duplicate()] = StoredFluid(liquid_mass, gas_mass);
	modify(tmp);
}

StoredFluid::StoredFluid()
{
	liquid_mass = 0.0f;
	gas_mass = 0.0f;
}

float StoredFluid::get_total_mass()
{
	return liquid_mass + gas_mass;
}

StoredFluid::StoredFluid(float liquid, float gas)
{
	liquid_mass = liquid;
	gas_mass = gas;
}
