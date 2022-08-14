#include "LuaNoise.h"
#include <FastNoiseC/FastNoise.h>
#include <glm/glm.hpp>


void LuaNoise::load_to(sol::table& table)
{
	// We need to create a lua context to obtain the FFI stuff,
	// this is a costly operation but thanksfully is done only once
	// per script so performance is not an issue

	sol::state_view sview(table.lua_state());

	sview.open_libraries(sol::lib::ffi);

	// Load all the FFI stuff
	sview.script("ffi.cdef[["
	"void fn_set_seed(struct FastNoise* fn, int seed);"
	"void fn_set_frequency(struct FastNoise* fn, double freq);"
  	"void fn_set_fractal_octaves(struct FastNoise* fn, int octaves);"
  	"void fn_set_fractal_gain(struct FastNoise* fn, double gain);"
	"void fn_set_fractal_lacunarity(struct FastNoise* fn, double lacunarity);"
	"void fn_set_fractal_type(struct FastNoise* fn, int fractal_type);"
	"void fn_set_interp(struct FastNoise* fn, int interp);"
	"double fn_value2(struct FastNoise* fn, double x, double y);"
	"double fn_value_fractal2(struct FastNoise* fn, double x, double y);"
	"double fn_perlin2(struct FastNoise* fn, double x, double y);"
	"double fn_perlin_fractal2(struct FastNoise* fn, double x, double y);"
	"double fn_simplex2(struct FastNoise* fn, double x, double y);"
	"double fn_simplex_fractal2(struct FastNoise* fn, double x, double y);"
	"double fn_cellular2(struct FastNoise* fn, double x, double y);"
	"double fn_cubic2(struct FastNoise* fn, double x, double y);"
	"double fn_cubic_fractal2(struct FastNoise* fn, int x, int y);"
	"void fn_gradient_perturb2(struct FastNoise* fn, double* x, double* y);"
	"void fn_gradient_perturb_fractal2(struct FastNoise* fn, double* x, double* y);"
	"double fn_value3(struct FastNoise* fn, double x, double y, double z);"
	"double fn_value_fractal3(struct FastNoise* fn, double x, double y, double z);"
	"double fn_perlin3(struct FastNoise* fn, double x, double y, double z);"
	"double fn_perlin_fractal3(struct FastNoise* fn, double x, double y, double z);"
	"double fn_simplex3(struct FastNoise* fn, double x, double y, double z);"
	"double fn_simplex_fractal3(struct FastNoise* fn, double x, double y, double z);"
	"double fn_cellular3(struct FastNoise* fn, double x, double y, double z);"
	"double fn_cubic3(struct FastNoise* fn, double x, double y, double z);"
	"double fn_cubic_fractal3(struct FastNoise* fn, int x, int y, int z);"
	"void fn_gradient_perturb3(struct FastNoise* fn, double* x, double* y, double* z);"
	"void fn_gradient_perturb_fractal3(struct FastNoise* fn, double* x, double* y, double* z);"
	"double fn_simplex4(struct FastNoise* fn, double x, double y, double z, double w);"
 	"void fn_set_crater_chance(struct FastNoise* fn, double chance);"
    "void fn_set_crater_layers(struct FastNoise* fn, int layers);"
 	"double fn_crater3(struct FastNoise* fn, int do_rad, double x, double y, double z);"
  	"double fn_crater3_get_rad(struct FastNoise* fn);"
	"struct FastNoise* fn_new(int seed);]]", "internal: LuaNoise");
	// Little macro to shorten a bit the code
#define EXPORT_FFI(ffi_name, table_name) table[table_name] = sview["ffi"]["C"][ffi_name]

	// Export the functions
	// (Uncomment as new stuff is ported over)
	EXPORT_FFI("fn_set_seed", "set_seed");
	EXPORT_FFI("fn_set_frequency", "set_frequency");
	EXPORT_FFI("fn_set_fractal_lacunarity", "set_fractal_lacunarity");
	EXPORT_FFI("fn_set_fractal_type", "set_fractal_type");
	EXPORT_FFI("fn_set_interp", "set_interp");
	EXPORT_FFI("fn_set_fractal_octaves", "set_fractal_octaves");
	EXPORT_FFI("fn_set_fractal_gain", "set_fractal_gain");

	EXPORT_FFI("fn_value2", "value2");
	EXPORT_FFI("fn_value_fractal2", "value_fractal2");
	EXPORT_FFI("fn_perlin2", "perlin2");
	EXPORT_FFI("fn_perlin_fractal2", "perlin_fractal2");
	EXPORT_FFI("fn_simplex2", "simplex2");
	EXPORT_FFI("fn_simplex_fractal2", "simplex_fractal2");
	//EXPORT_FFI("fn_cellular2", "cellular2");
	//EXPORT_FFI("fn_cubic2", "cubic2");
	//EXPORT_FFI("fn_cubic_fractal2", "cubic_fractal2");
	//EXPORT_FFI("fn_gradient_perturb2", "gradient_perturb2");
	//EXPORT_FFI("fn_gradient_perturb_fractal2", "gradient_perturb_fractal2");
	EXPORT_FFI("fn_value3", "value3");
	EXPORT_FFI("fn_value_fractal3", "value_fractal3");
	EXPORT_FFI("fn_perlin3", "perlin3");
	EXPORT_FFI("fn_perlin_fractal3", "perlin_fractal3");
	EXPORT_FFI("fn_simplex3", "simplex3");
	EXPORT_FFI("fn_simplex_fractal3", "simplex_fractal3");
	//EXPORT_FFI("fn_cellular3", "cellular3");
	//EXPORT_FFI("fn_cubic3", "cubic3");
	//EXPORT_FFI("fn_cubic_fractal3", "cubic_fractal3");
	//EXPORT_FFI("fn_gradient_perturb3", "gradient_perturb3");
	//EXPORT_FFI("fn_gradient_perturb_fractal3", "gradient_perturb_fractal3");
	EXPORT_FFI("fn_set_crater_chance", "set_crater_chance");
	EXPORT_FFI("fn_set_crater_layers", "set_crater_layers");
	EXPORT_FFI("fn_crater3_get_rad", "crater3_get_rad");
	EXPORT_FFI("fn_crater3", "crater3");
	EXPORT_FFI("fn_simplex4", "simplex4");
	EXPORT_FFI("fn_new", "new");

	// Unload ffi to avoid security risks 
	sview["ffi"] = sol::nil;


	struct FastNoiseDeleter
	{
		void operator()(FastNoise* fn) { fn_delete(fn); }
	};

	// Set the new function to return a smart pointer which auto-deletes
	//table.set_function("new", [](int seed)
	//{
	//	FastNoise* fn = fn_new(seed);
	//	return std::unique_ptr<FastNoise, FastNoiseDeleter>(fn);
	//});	


	table.new_enum("interp",
		"linear", FN_Linear,
		"hermite", FN_Hermite,
		"quintic", FN_Quintic);

	table.new_enum("fractal_type",
		"fbm", FN_FBM,
		"billow", FN_Billow,
		"rigid_multi", FN_RigidMulti);

	table.new_enum("cellular_distance", 
		"euclidean", FN_Euclidean,
		"manhattan", FN_Manhattan,
		"natural", FN_Natural);

	table.new_enum("cellular_return",
		"cell_value", FN_CellValue,
		"noise_lookup", FN_NoiseLookup,
		"distance", FN_Distance,
		"distance2", FN_Distance2,
		"distance2add", FN_Distance2Add,
		"distance2sub", FN_Distance2Sub,
		"distance2mul", FN_Distance2Mul,
		"distance2div", FN_Distance2Div);
	
}

