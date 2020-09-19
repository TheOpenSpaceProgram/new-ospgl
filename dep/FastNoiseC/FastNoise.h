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

#ifdef __cplusplus
};
#endif

#endif
