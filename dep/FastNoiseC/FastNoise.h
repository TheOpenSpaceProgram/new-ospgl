#ifndef FASTNOISEC_H
#define FASTNOISEC_H

#define FN_CELLULAR_INDEX_MAX 3

typedef double FN_DECIMAL;

#ifdef __cplusplus
extern "C" {
#endif

// This prevents the symbols from being removed by the linker
#ifdef _WIN32
#define NO_IGNORE __declspec(dllexport)
#else
#define NO_IGNORE /**/
#endif

enum FN_Interp
{ FN_Linear, FN_Hermite, FN_Quintic };	

enum FN_FractalType 
{ FN_FBM, FN_Billow, FN_RigidMulti };

enum FN_CellularDistanceFunction 
{ FN_Euclidean, FN_Manhattan, FN_Natural };

enum FN_CellularReturnType 
{ FN_CellValue, FN_NoiseLookup, FN_Distance, FN_Distance2, FN_Distance2Add, 
	FN_Distance2Sub, FN_Distance2Mul, FN_Distance2Div };


// A struct is needed to work on multiple thread
typedef struct FastNoise
{
	// Don't change these two, they are changed using fn_set_seed
	unsigned char perm[512];
	unsigned char perm12[512];

	int seed;
	FN_DECIMAL frequency;
	enum FN_Interp interp;

	// DO NOT CHANGE HERE, use fn_set_fractal_octaves
	int octaves;
	enum FN_FractalType fractal_type;
	FN_DECIMAL lacunarity;
	// DO NOT CHANGE HERE, use fn_set_fractal_gain
	FN_DECIMAL gain;
	FN_DECIMAL fractal_bounding;

	enum FN_CellularDistanceFunction cellular_distance_function;
	enum FN_CellularReturnType cellular_return_type;
	struct FastNoise* cellular_noise_lookup;
	
	int cellular_distance_index_0;
	int cellular_distance_index_1;
	FN_DECIMAL cellular_jitter;
	FN_DECIMAL gradient_perturb_amp;

	FN_DECIMAL crater_chance;
	int crater_layers;

	FN_DECIMAL crater_rad;

} FastNoise;


NO_IGNORE FastNoise* fn_new(int seed);
NO_IGNORE void fn_delete(FastNoise* fn);
NO_IGNORE void fn_set_seed(FastNoise* fn, int seed);
NO_IGNORE void fn_set_frequency(FastNoise* fn, FN_DECIMAL freq);
NO_IGNORE void fn_set_fractal_octaves(FastNoise* fn, int octaves);
NO_IGNORE void fn_set_fractal_gain(FastNoise* fn, FN_DECIMAL gain);
NO_IGNORE void fn_set_fractal_lacunarity(FastNoise* fn, FN_DECIMAL lacunarity);
NO_IGNORE void fn_set_fractal_type(FastNoise* fn, enum FN_FractalType fractal_type);
NO_IGNORE void fn_set_interp(FastNoise* fn, enum FN_Interp interp);

// Noise functions
NO_IGNORE FN_DECIMAL fn_value2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y);
NO_IGNORE FN_DECIMAL fn_value_fractal2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y);
NO_IGNORE FN_DECIMAL fn_perlin2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y);
NO_IGNORE FN_DECIMAL fn_perlin_fractal2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y);
NO_IGNORE FN_DECIMAL fn_simplex2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y);
NO_IGNORE FN_DECIMAL fn_simplex_fractal2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y);
NO_IGNORE FN_DECIMAL fn_cellular2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y);
NO_IGNORE FN_DECIMAL fn_cubic2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y);
NO_IGNORE FN_DECIMAL fn_cubic_fractal2(FastNoise* fn, int x, int y);
NO_IGNORE void fn_gradient_perturb2(FastNoise* fn, FN_DECIMAL* x, FN_DECIMAL* y);
NO_IGNORE void fn_gradient_perturb_fractal2(FastNoise* fn, FN_DECIMAL* x, FN_DECIMAL* y);

NO_IGNORE FN_DECIMAL fn_value3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z);
NO_IGNORE FN_DECIMAL fn_value_fractal3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z);
NO_IGNORE FN_DECIMAL fn_perlin3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z);
NO_IGNORE FN_DECIMAL fn_perlin_fractal3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z);
NO_IGNORE FN_DECIMAL fn_simplex3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z);
NO_IGNORE FN_DECIMAL fn_simplex_fractal3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z);
NO_IGNORE FN_DECIMAL fn_cellular3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z);
NO_IGNORE FN_DECIMAL fn_cubic3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z);
NO_IGNORE FN_DECIMAL fn_cubic_fractal3(FastNoise* fn, int x, int y, int z);
NO_IGNORE void fn_gradient_perturb3(FastNoise* fn, FN_DECIMAL* x, FN_DECIMAL* y, FN_DECIMAL* z);
NO_IGNORE void fn_gradient_perturb_fractal3(FastNoise* fn, FN_DECIMAL* x, FN_DECIMAL* y, FN_DECIMAL* z);

NO_IGNORE FN_DECIMAL fn_simplex4(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL w);

// CRATER NOISE
// (Custom method designed by Tatjam and not included in original FastNoise)
// An implementation of "crater" noise, returns "distance" and radial pos respect to a set of points
// randomly placed through a 3D grid. These points may never overlap as we use a grid method
// so only one crater per grid. Use multiple samples to generate interesting patterns

// Sets the chance of a grid location having a crater, from 0 to 1
// Note that changing crater chance will change the whole distribution of craters, don't
// use as a way to control craters locally! For that, multiply the craters by another noise map
NO_IGNORE void fn_set_crater_chance(FastNoise* fn, FN_DECIMAL chance);
// Set the number of passes to do with the crater generator.
// The grid and seed will be changed during the process to populate craters
// in a more natural way
// If you use the radial components, you may be better off composing manually
// so that overlapping craters are no issue
NO_IGNORE void fn_set_crater_layers(FastNoise* fn, int num);

// TODO: Profile if radial calculation boolean is a slow-down, so we could
// make two functions instead of the bool parameter

// Returns a value from 0 to 1 indicating clamped squared distance to the crater
// if calculate_rad is not false, the radial position respect to the crater is written
// to fn->crater_rad (This is used for easy lua interop without wrappers for speed!)
// As the noise function is 3D, radial position is respect to the plane (x, y, z)
// (Use this to make ejecta, etc...)
// It returns the square value to reduce minimum computation to the absolute minimum, if you
// want to use a linear function as crater shape, take the square root!
// To layer craters, use whichever method looks good, a good one is min(layer1, layer2)
NO_IGNORE FN_DECIMAL fn_crater3(FastNoise* fn, int calculate_rad, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z);
// Call after fn_crater3 to obtain the radial component (if it was calculated)
NO_IGNORE FN_DECIMAL fn_crater3_get_rad(FastNoise* fn);

#ifdef __cplusplus
};
#endif

#endif
