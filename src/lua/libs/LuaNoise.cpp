#include "LuaNoise.h"
#include <FastNoise/FastNoise.h>
#include <glm/glm.hpp>


void LuaNoise::load_to(sol::table & table)
{

	using NUM = FN_DECIMAL;

	// Enums:
	table.new_enum("noise_type",
		"value", FastNoise::NoiseType::Value,
		"value_fractal", FastNoise::NoiseType::ValueFractal,
		"perlin", FastNoise::NoiseType::Perlin,
		"perlin_fractal", FastNoise::NoiseType::PerlinFractal,
		"simplex", FastNoise::NoiseType::Simplex,
		"simplex_fractal", FastNoise::NoiseType::SimplexFractal,
		"cellular", FastNoise::NoiseType::Cellular,
		"white_noise", FastNoise::NoiseType::WhiteNoise,
		"cubic", FastNoise::NoiseType::Cubic,
		"cubic_fractal", FastNoise::NoiseType::CubicFractal);

	table.new_enum("interp",
		"linear", FastNoise::Interp::Linear,
		"hermite", FastNoise::Interp::Hermite,
		"quintic", FastNoise::Interp::Quintic);

	table.new_enum("fractal_type",
		"fbm", FastNoise::FractalType::FBM,
		"billow", FastNoise::FractalType::Billow,
		"rigid_multi", FastNoise::FractalType::RigidMulti);

	table.new_enum("cellular_distance", 
		"euclidean", FastNoise::Euclidean,
		"manhattan", FastNoise::Manhattan,
		"natural", FastNoise::Natural);

	table.new_enum("cellular_return",
		"cell_value", FastNoise::CellValue,
		"noise_lookup", FastNoise::NoiseLookup,
		"distance", FastNoise::Distance,
		"distance2", FastNoise::Distance2,
		"distance2add", FastNoise::Distance2Add,
		"distance2sub", FastNoise::Distance2Sub,
		"distance2mul", FastNoise::Distance2Mul,
		"distance2div", FastNoise::Distance2Div);


	sol::usertype<FastNoise> noise_ut = table.new_usertype<FastNoise>("noise",
		sol::constructors<FastNoise(int)>(),
		"set_seed", &FastNoise::SetSeed, "get_seed", &FastNoise::GetSeed,
		"set_frequency", &FastNoise::SetFrequency, "get_frequency", &FastNoise::GetFrequency,
		"set_interp", &FastNoise::SetInterp, "get_interp", &FastNoise::GetInterp,
		"set_noise_type", &FastNoise::SetNoiseType, "get_noise_type", &FastNoise::GetNoiseType,
		"set_fractal_octaves", &FastNoise::SetFractalOctaves, "get_fractal_octaves", &FastNoise::GetFractalOctaves,
		"set_fractal_lacunarity", &FastNoise::SetFractalLacunarity, "get_fractal_lacunarity", &FastNoise::GetFractalLacunarity,
		"set_fractal_gain", &FastNoise::SetFractalGain, "get_fractal_gain", &FastNoise::GetFractalGain,
		"set_fractal_type", &FastNoise::SetFractalType, "get_fractal_type", &FastNoise::GetFractalType,
		"set_cellular_distance", &FastNoise::SetCellularDistanceFunction, "get_cellular_distance", &FastNoise::GetCellularDistanceFunction,
		"set_cellular_return", &FastNoise::SetCellularReturnType, "get_cellular_return", &FastNoise::GetCellularReturnType,
		"set_cellular_distance2_indices", &FastNoise::SetCellularDistance2Indices,
		"get_cellular_distance2_indices", [](FastNoise& n)
		{
			int a, b;
			n.GetCellularDistance2Indices(a, b);
			return std::make_tuple(a, b);
		},
		"set_cellular_jitter", &FastNoise::SetCellularJitter,
		"get_cellular_jitter", &FastNoise::GetCellularJitter,
		"set_gradient_perturb_amp", &FastNoise::SetGradientPerturbAmp,
		"get_gradient_perturb_amp", &FastNoise::GetGradientPerturbAmp,

		// Noise functions
		"get_value", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetValue),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetValue),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetValue(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetValue(v.x, v.y, v.z); }
		),

		"get_value_fractal", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetValueFractal),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetValueFractal),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetValueFractal(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetValueFractal(v.x, v.y, v.z); }
		),

		"get_perlin", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetPerlin),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetPerlin),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetPerlin(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetPerlin(v.x, v.y, v.z); }
		),

		"get_perlin_fractal", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetPerlinFractal),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetPerlinFractal),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetPerlinFractal(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetPerlinFractal(v.x, v.y, v.z); }
		),

		"get_simplex", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetSimplex),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetSimplex),
			sol::resolve<NUM(NUM, NUM, NUM, NUM) const>(&FastNoise::GetSimplex),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetSimplex(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetSimplex(v.x, v.y, v.z); },
			[](const FastNoise& n, glm::dvec4 v) {return n.GetSimplex(v.x, v.y, v.z, v.w); }
		),

		"get_simplex_fractal", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetSimplexFractal),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetSimplexFractal),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetSimplexFractal(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetSimplexFractal(v.x, v.y, v.z); }
		),

		"get_cellular", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetCellular),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetCellular),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetCellular(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetCellular(v.x, v.y, v.z); }
		),

		"get_white_noise", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetWhiteNoise),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetWhiteNoise),
			sol::resolve<NUM(NUM, NUM, NUM, NUM) const>(&FastNoise::GetWhiteNoise),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetWhiteNoise(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetWhiteNoise(v.x, v.y, v.z); },
			[](const FastNoise& n, glm::dvec4 v) {return n.GetWhiteNoise(v.x, v.y, v.z, v.w); }
		),

		// Lua does not support ints, so it will simply cast the input values
		"get_white_noise_int", sol::overload(
			sol::resolve<NUM(int, int) const>(&FastNoise::GetWhiteNoiseInt),
			sol::resolve<NUM(int, int, int) const>(&FastNoise::GetWhiteNoiseInt),
			sol::resolve<NUM(int, int, int, int) const>(&FastNoise::GetWhiteNoiseInt),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetWhiteNoiseInt((int)v.x, (int)v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetWhiteNoiseInt((int)v.x, (int)v.y, (int)v.z); },
			[](const FastNoise& n, glm::dvec4 v) {return n.GetWhiteNoiseInt((int)v.x, (int)v.y, (int)v.z, (int)v.w); }
		),

		"get_cubic", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetCubic),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetCubic),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetCubic(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetCubic(v.x, v.y, v.z); }
		),

		"get_cubic_fractal", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetCubicFractal),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetCubicFractal),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetCubicFractal(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetCubicFractal(v.x, v.y, v.z); }
		),

		"get_noise", sol::overload(
			sol::resolve<NUM(NUM, NUM) const>(&FastNoise::GetNoise),
			sol::resolve<NUM(NUM, NUM, NUM) const>(&FastNoise::GetNoise),
			[](const FastNoise& n, glm::dvec2 v) {return n.GetNoise(v.x, v.y); },
			[](const FastNoise& n, glm::dvec3 v) {return n.GetNoise(v.x, v.y, v.z); }
		),

		"gradient_perturb", sol::overload(

			[](const FastNoise& n, NUM a, NUM b) 
			{
				n.GradientPerturb(a, b);
				return std::make_tuple(a, b);
			},

			[](const FastNoise& n, NUM a, NUM b, NUM c)
			{
				n.GradientPerturb(a, b, c);
				return std::make_tuple(a, b, c);
			},

			[](const FastNoise& n, glm::dvec2 v)
			{
				n.GradientPerturb(v.x, v.y);
				return v;
			},

			[](const FastNoise& n, glm::dvec3 v)
			{
				n.GradientPerturb(v.x, v.y, v.z);
				return v;
			}
		)

		);

	table.set_function("new", table["noise"].get<sol::table>()["new"].get<sol::function>());
	

}

LuaNoise::LuaNoise()
{
}


LuaNoise::~LuaNoise()
{
}
