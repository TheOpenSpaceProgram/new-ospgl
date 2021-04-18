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

StoredFluid::StoredFluid()
{
	liquid_mass = 0.0f;
	gas_mass = 0.0f;
}

float StoredFluid::get_total_mass()
{
	return liquid_mass + gas_mass;
}
