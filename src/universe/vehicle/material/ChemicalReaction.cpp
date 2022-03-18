#include "ChemicalReaction.h"
#include <assets/AssetManager.h>
#include <assets/PhysicalMaterial.h>

void ChemicalReaction::calculate_constants()
{
	float total_h = 0.0f;
	float total_s = 0.0F;
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
			total_s += mat->S * weight;
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
	dS = total_s / total_mass;
}

float ChemicalReaction::get_gibbs_free_energy(float T) const
{
	return dH - dS * T;
}

void ChemicalReaction::react(StoredFluids* fluids, float ammount)
{

}

float ChemicalReaction::get_rate(float T)
{
	return 0;
}
