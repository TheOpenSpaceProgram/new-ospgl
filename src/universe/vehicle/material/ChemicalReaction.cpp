#include "ChemicalReaction.h"
#include <assets/AssetManager.h>
#include <assets/PhysicalMaterial.h>
#include "../plumbing/StoredFluids.h"

void ChemicalReaction::calculate_constants()
{
	float total_h = 0.0f;
	// Total mass of reactants. Should be "equal" to mass of products
	// within a small margin of error
	float total_mass = 0.0f;
	float total_prod_mass = 0.0f;

	average_cP = 0.0f;
	float tot = 0.0f;

	// Note that stechiometry is given in moles but the values are in kg! We need to
	// convert the moles to kgs!
	for(StechiometricMaterial& r : reactants)
	{
		auto mat = AssetHandle<PhysicalMaterial>(r.reactant);

		float weight = r.moles * mat->molar_mass;
		average_cP += mat->heat_capacity_gas * abs(weight);
		tot += weight;

		if(r.moles > 0)
		{
			total_h += mat->dH_formation * weight;
		}

		if(weight > 0.0f)
		{
			total_mass += weight;
		}
		else
		{
			total_prod_mass -= weight;
		}
	}

	float dmass = total_mass - total_prod_mass;
	// We allow a small wiggle-room for molecular weights as we simulate rockets
	// and not microscale chemistry
	if(abs(dmass) > 0.0001f)
	{
		logger->error("Reaction is badly balanced. Mass difference (reactants - products): {}", dmass);
	}

	for(StechiometricMaterial& r : reactants)
	{
		auto mat = AssetHandle<PhysicalMaterial>(r.reactant);
		float weight = r.moles * mat->molar_mass;
		r.react_weight = weight / total_mass;
	}

	average_cP /= tot;

	dH = total_h / total_mass;
}

float ChemicalReaction::react(StoredFluids* fluids, float gas_ammount, float liquid_ammount)
{
	// Limiting reagent check
	for(const auto& rct : reactants)
	{
		const PhysicalMaterial* mat = fluids->name_to_ptr[rct.reactant];
		// Only react as much as we can (limiting reagent, adjust)
		if(gas_ammount > 0 || liquid_ammount > 0)
		{
			// We react to the right and thus reactants are limiting
			if(rct.react_weight > 0 && fluids->contents[mat].gas_mass < rct.react_weight * gas_ammount)
			{
				gas_ammount = fluids->contents[mat].gas_mass / rct.react_weight;
			}
			if(rct.react_weight > 0 && fluids->contents[mat].liquid_mass < rct.react_weight * liquid_ammount)
			{
				liquid_ammount = fluids->contents[mat].liquid_mass / rct.react_weight;
			}
		}
		else
		{
			// We react to the left and thus products are limiting (reaction is inversed)
			// Careful with the signs!
			if(rct.react_weight < 0 && fluids->contents[mat].gas_mass < rct.react_weight * gas_ammount)
			{
				gas_ammount = fluids->contents[mat].gas_mass / rct.react_weight;
			}
			if(rct.react_weight < 0 && fluids->contents[mat].liquid_mass < rct.react_weight * liquid_ammount)
			{
				liquid_ammount = fluids->contents[mat].liquid_mass / rct.react_weight;
			}
		}
	}

	for(const auto& rct : reactants)
	{
		const PhysicalMaterial* mat = fluids->name_to_ptr[rct.reactant];
		fluids->contents[mat].gas_mass -= rct.react_weight * gas_ammount;
		fluids->contents[mat].liquid_mass -= rct.react_weight * liquid_ammount;
	}

	return gas_ammount + liquid_ammount;
}

float ChemicalReaction::get_rate(float T)
{
	return 1.0f;
}

float ChemicalReaction::get_Q(StoredFluids* fluids, float V)
{
	// Concentrations of everything is mol / L
	float Q = 1.0f;

	for(auto r : reactants)
	{
		auto mat = fluids->name_to_ptr[r.reactant];
		float moles = fluids->contents[mat].gas_mass + fluids->contents[mat].liquid_mass;
		moles /= mat->molar_mass;
		if(glm::abs(moles) < 0.001f)
		{
			// To prevent singularity when there's nothing of a product
			moles = 0.00001f;
		}
		Q *= powf(moles / V, r.moles);
	}

	return Q;
}

float ChemicalReaction::get_K(float T)
{
	constexpr float R = 8.314462618f;
	// k2 = k1 * e^((dH / R) * (1 / T1 - 1 / T2))
	return K * expf((dH / R) * (1.0f / T_of_K - 1 / T));
}
