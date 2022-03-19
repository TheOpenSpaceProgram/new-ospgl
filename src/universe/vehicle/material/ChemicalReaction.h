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
// We use the approximation that dS and dH are constant with temperature and thus
// we can approximate the equlibrium at any temperature as:
// ln(k2) - ln(k1) = (dH / R) * (1 / T1 - 1 / T2)
// This way we avoid the need for inputting dS for materials, but add the task
// of obtaining an equilibrium constant at a given temperature for reactions
// For one-way only directions, you may use a really big k value to get a good approx
class ChemicalReaction
{
public:

	std::vector<StechiometricMaterial> reactants;

	// precalculated values
	float dH, average_cP, K, T_of_K;

	// Gets the rate of the reaction given temperature of the mixture
	// Rate in kg / (s * m^3), don't forget to account for the reaction volume
	float get_rate(float T);


	// Very approximate values!
	void calculate_constants();

	// Ammount refers to the total kgs of reactant that convert
	// Returns total ammount converted
	float react(StoredFluids* fluids, float gas_ammount, float liquid_ammount);

	float get_Q(StoredFluids* fluids, float V);
	float get_K(float T);

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
		SAFE_TOML_GET(to.K, "K", double);
		SAFE_TOML_GET(to.T_of_K, "T_of_K", double);
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


