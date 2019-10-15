#pragma once
#include <sol.hpp>
#include <FastNoise/FastNoise.h>

class LuaNoiseLib
{
public:

	static void load_lib(sol::state& lua_state, FastNoise* noise)
	{
		lua_state["noise"] = lua_state.create_table();

		// Noise library
		lua_state["noise"]["perlin3"] = [noise](FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
		{
			return noise->GetPerlin(x, y, z);
		};

		lua_state["noise"]["perlin2"] = [noise](FN_DECIMAL x, FN_DECIMAL y)
		{
			return noise->GetPerlin(x, y);
		};

		lua_state["noise"]["perlin3_fractal"] = [noise](FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
		{
			return noise->GetPerlinFractal(x, y, z);
		};

		lua_state["noise"]["perlin2_fractal"] = [noise](FN_DECIMAL x, FN_DECIMAL y)
		{
			return noise->GetPerlinFractal(x, y);
		};


		lua_state["noise"]["value3"] = [noise](FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
		{
			return noise->GetValue(x, y, z);
		};

		lua_state["noise"]["value2"] = [noise](FN_DECIMAL x, FN_DECIMAL y)
		{
			return noise->GetValue(x, y);
		};

		lua_state["noise"]["value3_fractal"] = [noise](FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
		{
			return noise->GetValueFractal(x, y, z);
		};

		lua_state["noise"]["value2_fractal"] = [noise](FN_DECIMAL x, FN_DECIMAL y)
		{
			return noise->GetValueFractal(x, y);
		};

		lua_state["noise"]["simplex4"] = [noise](FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL w)
		{
			return noise->GetSimplex(x, y, z, w);
		};

		lua_state["noise"]["simplex3"] = [noise](FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
		{
			return noise->GetSimplex(x, y, z);
		};

		lua_state["noise"]["simplex2"] = [noise](FN_DECIMAL x, FN_DECIMAL y)
		{
			return noise->GetSimplex(x, y);
		};

		lua_state["noise"]["simplex3_fractal"] = [noise](FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
		{
			return noise->GetSimplexFractal(x, y, z);
		};

		lua_state["noise"]["simplex2_fractal"] = [noise](FN_DECIMAL x, FN_DECIMAL y)
		{
			return noise->GetSimplexFractal(x, y);
		};


		lua_state["noise"]["cellular3"] = [noise](FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
		{
			return noise->GetCellular(x, y, z);
		};

		lua_state["noise"]["cubic3"] = [noise](FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
		{
			return noise->GetCubic(x, y, z);
		};

		lua_state["noise"]["cubic3_fractal"] = [noise](FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
		{
			return noise->GetCubicFractal(x, y, z);
		};

		lua_state["noise"]["set_frequency"] = [noise](FN_DECIMAL freq)
		{
			noise->SetFrequency(freq);
		};


		lua_state["noise"]["set_fractal_gain"] = [noise](FN_DECIMAL gain)
		{
			noise->SetFractalGain(gain);
		};

		lua_state["noise"]["set_fractal_lacunarity"] = [noise](FN_DECIMAL lacunarity)
		{
			noise->SetFractalLacunarity(lacunarity);
		};

		lua_state["noise"]["set_fractal_octaves"] = [noise](int octaves)
		{
			noise->SetFractalOctaves(octaves);
		};


		lua_state["noise"]["set_fractal_fbm"] = [noise]()
		{
			noise->SetFractalType(FastNoise::FractalType::FBM);
		};

		lua_state["noise"]["set_fractal_billow"] = [noise]()
		{
			noise->SetFractalType(FastNoise::FractalType::Billow);
		};

		lua_state["noise"]["set_fractal_rigidmulti"] = [noise]()
		{
			noise->SetFractalType(FastNoise::FractalType::RigidMulti);
		};

		lua_state["noise"]["set_cellular_distance"] = [noise]()
		{
			noise->SetCellularReturnType(FastNoise::CellularReturnType::Distance);
		};

		lua_state["noise"]["set_cellular_value"] = [noise]()
		{
			noise->SetCellularReturnType(FastNoise::CellularReturnType::CellValue);
		};

		lua_state["noise"]["set_cellular_distance"] = [noise]()
		{
			noise->SetCellularReturnType(FastNoise::CellularReturnType::Distance);
		};

		lua_state["noise"]["set_cellular_distance2"] = [noise]()
		{
			noise->SetCellularReturnType(FastNoise::CellularReturnType::Distance2);
		};

		lua_state["noise"]["set_cellular_distance2add"] = [noise]()
		{
			noise->SetCellularReturnType(FastNoise::CellularReturnType::Distance2Add);
		};

		lua_state["noise"]["set_cellular_distance2sub"] = [noise]()
		{
			noise->SetCellularReturnType(FastNoise::CellularReturnType::Distance2Sub);
		};

		lua_state["noise"]["set_cellular_distance2mul"] = [noise]()
		{
			noise->SetCellularReturnType(FastNoise::CellularReturnType::Distance2Mul);
		};

		lua_state["noise"]["set_cellular_distance2div"] = [noise]()
		{
			noise->SetCellularReturnType(FastNoise::CellularReturnType::Distance2Div);
		};
	}
	
};