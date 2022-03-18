#pragma once
#include <util/SerializeUtil.h>

struct StechiometricMaterial
{
	// Negative if it's on the right hand side
	int moles;
	std::string reactant;

	// Calculated from stechiometry, how much weight does this material have on the
	// total reaction mass? (percent)
	float react_weight;
};

class StoredFluids;

// Note: We do simulate reversible chemical reactions! We use gibbs free energy for
// this, and thus all reactions can be reversed!
class ChemicalReaction
{
public:

	std::vector<StechiometricMaterial> reactants;
	float eq_constant;

	// precalculated values
	float dH, dS, average_cP;


	float get_gibbs_free_energy(float T) const;
	// Gets the rate of the reaction given temperature of the mixture
	// Rate in kg / (s * m^3), don't forget to account for the reaction volume
	float get_rate(float T);

	// Very approximate values!
	void calculate_constants();

	// Ammount refers to the total kgs of reactant that convert
	void react(StoredFluids* fluids, float ammount);

};

static bool operator==(const ChemicalReaction& a, const ChemicalReaction& b)
{
	if(a.reactants.size() != b.reactants.size())
	{
		return false;
	}

	// They must be ordered
	for(size_t i = 0; i < a.reactants.size(); i++)
	{
		if(a.reactants[i].moles != b.reactants[i].moles ||
			a.reactants[i].reactant != b.reactants[i].reactant)
		{
			return false;
		}
	}

	return true;
}


template<>
class GenericSerializer<ChemicalReaction>
{
public:
	static void serialize(const ChemicalReaction& what, cpptoml::table& target)
	{
		logger->fatal("NOT IMPLEMENTED");
	}

	static void deserialize(ChemicalReaction& to, const cpptoml::table& from)
	{
		// Now get the array of reactants and their stechiometry
		std::vector<int> stechiometry;
		auto st_arr = *from.get_array_of<int64_t>("stechiometry");
		auto rt_arr = *from.get_array_of<std::string>("reactants");

		logger->check(st_arr.size() == rt_arr.size(), "Unmatching reactants and stechiometry");
		for(size_t i = 0; i < st_arr.size(); i++)
		{
			StechiometricMaterial mat;
			mat.reactant = rt_arr[i];
			mat.moles = st_arr[i];
			to.reactants.push_back(mat);
		}

		to.calculate_constants();
	}
};


