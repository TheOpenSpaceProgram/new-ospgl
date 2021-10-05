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
			for(auto& ast : b.mat_ptrs)
			{
				if(ast.data == pair.first)
				{
					mat_ptrs.insert(ast.duplicate());
				}
			}

			contents[pair.first] = pair.second;
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
			out.contents[pair.first] = StoredFluid();
			// We create a new AssetHandle in out to store it as we are creating the key
			for(auto& ast : b.mat_ptrs)
			{
				if(ast.data == pair.first)
				{
					out.mat_ptrs.insert(ast.duplicate());
				}
			}

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
		out.contents[pair.first] = pair.second;
		// we create a new assethandle in out to store it as we are creating the key
		for(auto& ast : mat_ptrs)
		{
			if(ast.data == pair.first)
			{
				out.mat_ptrs.insert(ast.duplicate());
			}
		}
		out.contents.at(pair.first).gas_mass *= value;
		out.contents.at(pair.first).liquid_mass *= value;
	}

	return out;
}

void StoredFluids::remove_empty_fluids()
{

}

void StoredFluids::add_fluid(const AssetHandle<PhysicalMaterial>& mat, float liquid_mass, float gas_mass)
{
	StoredFluids tmp;
	if(mat_ptrs.count(mat) == 0)
	{
		// We must introduce a new asset handle
		mat_ptrs.insert(mat.duplicate());
	}

	tmp.contents[mat.data] = StoredFluid(liquid_mass, gas_mass);
	modify(tmp);
}

void StoredFluids::drain_to(StoredFluids* target, PhysicalMaterial* mat, float liquid_mass, float gas_mass, bool do_flow)
{
	auto it = target->contents.find(mat);
	StoredFluid* target_fluids;
	if(it == target->contents.end())
	{
		// we create a new assethandle in out to store it as we are creating the key
		for(auto& ast : mat_ptrs)
		{
			if(ast.data == mat)
			{
				target->mat_ptrs.insert(ast.duplicate());
			}
		}

		target->contents[mat] = StoredFluid();
		target_fluids = &target->contents[mat];
	}
	else
	{
		target_fluids = &it->second;
	}

	// Drain from our tank
	auto self_it = contents.find(mat);
	logger->check(self_it != contents.end(), "Could not find material in drain_to, this is not allowed!");

	float tmp_liquid = self_it->second.liquid_mass;
	float tmp_gas = self_it->second.gas_mass;

	self_it->second.gas_mass -= gas_mass;
	self_it->second.liquid_mass -= liquid_mass;
	float tfer_gas = gas_mass;
	float tfer_liq = liquid_mass;
	if(self_it->second.liquid_mass < 0.0f)
	{
		tfer_liq = -self_it->second.liquid_mass;
	}
	if(self_it->second.gas_mass < 0.0f)
	{
		tfer_gas = -self_it->second.gas_mass;
	}

	self_it->second.gas_mass = glm::max(self_it->second.gas_mass, 0.0f);
	self_it->second.liquid_mass = glm::max(self_it->second.liquid_mass, 0.0f);

	target_fluids->gas_mass += tfer_gas;
	target_fluids->liquid_mass += tfer_liq;

	// Restore the original fluids
	if(!do_flow)
	{
		self_it->second.gas_mass = tmp_gas;
		self_it->second.liquid_mass = tmp_liquid;
	}

	remove_empty_fluids();

}

float StoredFluids::get_total_liquid_mass() const
{
	float total = 0.0f;
	for(const auto& pair : contents)
	{
		total += pair.second.liquid_mass;
	}
	return total;
}

float StoredFluids::get_total_liquid_volume() const
{
	float total = 0.0f;
	for(const auto& pair : contents)
	{
		total += pair.second.liquid_mass / pair.first->liquid_density;
	}
	return total;
}

float StoredFluids::get_average_liquid_density() const
{
	return get_total_liquid_mass() / get_total_liquid_volume();
}

float StoredFluids::get_total_gas_mass() const
{
	float total = 0.0f;
	for(const auto& pair : contents)
	{
		total += pair.second.gas_mass;
	}
	return total;
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
