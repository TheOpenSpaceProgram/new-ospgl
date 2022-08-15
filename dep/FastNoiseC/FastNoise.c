#include "FastNoise.h"
#include "FastNoiseArrays.h"
#include <stdint.h>
#include <math.h>
#include <stdlib.h>

#define RND_IMPLEMENTATION
#include "rnd.h"

static int FastFloor(FN_DECIMAL f) { return (f >= 0 ? (int)f : (int)f - 1); }
static int FastRound(FN_DECIMAL f) { return (f >= 0) ? (int)(f + (FN_DECIMAL)(0.5)) : (int)(f - (FN_DECIMAL)(0.5)); }
static int FastAbs(int i) { return abs(i); }
static FN_DECIMAL FastAbsf(FN_DECIMAL f) { return fabs(f); }
static FN_DECIMAL Lerp(FN_DECIMAL a, FN_DECIMAL b, FN_DECIMAL t) { return a + t * (b - a); }
static FN_DECIMAL InterpHermiteFunc(FN_DECIMAL t) { return t * t*(3 - 2 * t); }
static FN_DECIMAL InterpQuinticFunc(FN_DECIMAL t) { return t * t*t*(t*(t * 6 - 15) + 10); }
static FN_DECIMAL CubicLerp(FN_DECIMAL a, FN_DECIMAL b, FN_DECIMAL c, FN_DECIMAL d, FN_DECIMAL t)
{
	FN_DECIMAL p = (d - c) - (a - b);
	return t * t * t * p + t * t * ((a - b) - p) + t * (c - a) + b;
}

void fn_set_seed(FastNoise* fn, int seed)
{
	fn->seed = seed;
	rnd_pcg_t pcg;
	rnd_pcg_seed(&pcg, (unsigned int)(seed));

	for (int i = 0; i < 256; i++)
		fn->perm[i] = i;

	for (int j = 0; j < 256; j++)
	{
		int rng = (int)(rnd_pcg_next(&pcg) % (256 - j));
		int k = rng + j;
		int l = fn->perm[j];
		fn->perm[j] = fn->perm[j + 256] = fn->perm[k];
		fn->perm[k] = l;
		fn->perm12[j] = fn->perm12[j + 256] = fn->perm[j] % 12;
	}
}


static void CalculateFractalBounding(FastNoise* fn)
{
	FN_DECIMAL amp = fn->gain;
	FN_DECIMAL ampFractal = 1.0f;
	for (int i = 1; i < fn->octaves; i++)
	{
		ampFractal += amp;
		amp *= fn->gain;
	}
	fn->fractal_bounding = 1.0f / ampFractal;
}

void fn_set_fractal_octaves(FastNoise* fn, int octaves)
{
	fn->octaves = octaves;
	CalculateFractalBounding(fn);
}

void fn_set_fractal_gain(FastNoise* fn, FN_DECIMAL gain)
{
	fn->gain = gain;
	CalculateFractalBounding(fn);
}

void fn_set_frequency(FastNoise* fn, FN_DECIMAL freq)
{
	fn->frequency = freq;
}

void fn_set_fractal_lacunarity(FastNoise* fn, FN_DECIMAL lacunarity)
{
	fn->lacunarity = lacunarity;
}

void fn_set_fractal_type(FastNoise* fn, enum FN_FractalType fractal_type)
{
	fn->fractal_type = fractal_type;
}

void fn_set_interp(FastNoise* fn, enum FN_Interp interp)
{
	fn->interp = interp;
}

FastNoise* fn_new(int seed)
{
	FastNoise* fn = (FastNoise*)malloc(sizeof(FastNoise));	

	fn->frequency = (FN_DECIMAL)(0.01);
	fn->interp = FN_Quintic;
	fn->octaves = 3;
	fn->lacunarity = (FN_DECIMAL)(2);
	fn->gain = (FN_DECIMAL)(0.5);
	fn->fractal_type = FN_FBM;
	fn->cellular_distance_function = FN_Euclidean;	
	fn->cellular_return_type = FN_CellValue;
	fn->cellular_distance_index_0 = 0;
	fn->cellular_distance_index_1 = 1;
	fn->cellular_jitter = (FN_DECIMAL)(1);
	fn->gradient_perturb_amp = (FN_DECIMAL)(1);

	fn_set_seed(fn, seed);

	CalculateFractalBounding(fn);

	return fn;
}

void fn_delete(FastNoise* fn)
{
	free(fn);
}

static unsigned char Index2D_12(FastNoise* fn, unsigned char offset, int x, int y)
{
	return fn->perm12[(x & 0xff) + fn->perm[(y & 0xff) + offset]];
}
static unsigned char Index3D_12(FastNoise* fn, unsigned char offset, int x, int y, int z)
{
	return fn->perm12[(x & 0xff) + fn->perm[(y & 0xff) + fn->perm[(z & 0xff) + offset]]];
}
static unsigned char Index4D_32(FastNoise* fn, unsigned char offset, int x, int y, int z, int w)
{
	return fn->perm[(x & 0xff) + fn->perm[(y & 0xff) + fn->perm[(z & 0xff) + fn->perm[(w & 0xff) + offset]]]] & 31;
}
static unsigned char Index2D_256(FastNoise* fn, unsigned char offset, int x, int y)
{
	return fn->perm[(x & 0xff) + fn->perm[(y & 0xff) + offset]];
}
static unsigned char Index3D_256(FastNoise* fn, unsigned char offset, int x, int y, int z)
{
	return fn->perm[(x & 0xff) + fn->perm[(y & 0xff) + fn->perm[(z & 0xff) + offset]]];
}
static unsigned char Index4D_256(FastNoise* fn, unsigned char offset, int x, int y, int z, int w)
{
	return fn->perm[(x & 0xff) + fn->perm[(y & 0xff) + fn->perm[(z & 0xff) + fn->perm[(w & 0xff) + offset]]]];
}

// Hashing
#define X_PRIME 1619
#define Y_PRIME 31337
#define Z_PRIME 6971
#define W_PRIME 1013

static FN_DECIMAL ValCoord2D(int seed, int x, int y)
{
	int n = seed;
	n ^= X_PRIME * x;
	n ^= Y_PRIME * y;

	return (n * n * n * 60493) / (FN_DECIMAL)(2147483648);
}
static FN_DECIMAL ValCoord3D(int seed, int x, int y, int z)
{
	int n = seed;
	n ^= X_PRIME * x;
	n ^= Y_PRIME * y;
	n ^= Z_PRIME * z;

	return (n * n * n * 60493) / (FN_DECIMAL)(2147483648);
}
static FN_DECIMAL ValCoord4D(int seed, int x, int y, int z, int w)
{
	int n = seed;
	n ^= X_PRIME * x;
	n ^= Y_PRIME * y;
	n ^= Z_PRIME * z;
	n ^= W_PRIME * w;

	return (n * n * n * 60493) / (FN_DECIMAL)(2147483648);
}

FN_DECIMAL ValCoord2DFast(FastNoise* fn, unsigned char offset, int x, int y)
{
	return VAL_LUT[Index2D_256(fn, offset, x, y)];
}
FN_DECIMAL ValCoord3DFast(FastNoise* fn, unsigned char offset, int x, int y, int z)
{
	return VAL_LUT[Index3D_256(fn, offset, x, y, z)];
}

FN_DECIMAL GradCoord2D(FastNoise* fn, unsigned char offset, int x, int y, FN_DECIMAL xd, FN_DECIMAL yd)
{
	unsigned char lutPos = Index2D_12(fn, offset, x, y);

	return xd * GRAD_X[lutPos] + yd * GRAD_Y[lutPos];
}
FN_DECIMAL GradCoord3D(FastNoise* fn, unsigned char offset, int x, int y, int z, FN_DECIMAL xd, FN_DECIMAL yd, FN_DECIMAL zd)
{
	unsigned char lutPos = Index3D_12(fn, offset, x, y, z);

	return xd * GRAD_X[lutPos] + yd * GRAD_Y[lutPos] + zd * GRAD_Z[lutPos];
}
FN_DECIMAL GradCoord4D(FastNoise* fn, unsigned char offset, int x, int y, int z, int w, 
		FN_DECIMAL xd, FN_DECIMAL yd, FN_DECIMAL zd, FN_DECIMAL wd)
{
	unsigned char lutPos = Index4D_32(fn, offset, x, y, z, w) << 2;

	return xd * GRAD_4D[lutPos] + yd * GRAD_4D[lutPos + 1] + zd * GRAD_4D[lutPos + 2] + wd * GRAD_4D[lutPos + 3];
}

static FN_DECIMAL SingleValue3(FastNoise* fn, unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	int x0 = FastFloor(x);
	int y0 = FastFloor(y);
	int z0 = FastFloor(z);
	int x1 = x0 + 1;
	int y1 = y0 + 1;
	int z1 = z0 + 1;

	FN_DECIMAL xs, ys, zs;
	switch (fn->interp)
	{
	case FN_Linear:
		xs = x - (FN_DECIMAL)x0;
		ys = y - (FN_DECIMAL)y0;
		zs = z - (FN_DECIMAL)z0;
		break;
	case FN_Hermite:
		xs = InterpHermiteFunc(x - (FN_DECIMAL)x0);
		ys = InterpHermiteFunc(y - (FN_DECIMAL)y0);
		zs = InterpHermiteFunc(z - (FN_DECIMAL)z0);
		break;
	case FN_Quintic:
		xs = InterpQuinticFunc(x - (FN_DECIMAL)x0);
		ys = InterpQuinticFunc(y - (FN_DECIMAL)y0);
		zs = InterpQuinticFunc(z - (FN_DECIMAL)z0);
		break;
	}

	FN_DECIMAL xf00 = Lerp(ValCoord3DFast(fn, offset, x0, y0, z0), ValCoord3DFast(fn, offset, x1, y0, z0), xs);
	FN_DECIMAL xf10 = Lerp(ValCoord3DFast(fn, offset, x0, y1, z0), ValCoord3DFast(fn, offset, x1, y1, z0), xs);
	FN_DECIMAL xf01 = Lerp(ValCoord3DFast(fn, offset, x0, y0, z1), ValCoord3DFast(fn, offset, x1, y0, z1), xs);
	FN_DECIMAL xf11 = Lerp(ValCoord3DFast(fn, offset, x0, y1, z1), ValCoord3DFast(fn, offset, x1, y1, z1), xs);

	FN_DECIMAL yf0 = Lerp(xf00, xf10, ys);
	FN_DECIMAL yf1 = Lerp(xf01, xf11, ys);

	return Lerp(yf0, yf1, zs);
}

static FN_DECIMAL SingleValueFractalFBM3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	FN_DECIMAL sum = SingleValue3(fn, fn->perm[0], x, y, z);
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;
		z *= fn->lacunarity;

		amp *= fn->gain;
		sum += SingleValue3(fn, fn->perm[i], x, y, z) * amp;
	}

	return sum * fn->fractal_bounding;
}

static FN_DECIMAL SingleValueFractalBillow3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	FN_DECIMAL sum = FastAbs(SingleValue3(fn, fn->perm[0], x, y, z)) * 2 - 1;
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;
		z *= fn->lacunarity;

		amp *= fn->gain;
		sum += (FastAbs(SingleValue3(fn, fn->perm[i], x, y, z)) * 2 - 1) * amp;
	}

	return sum * fn->fractal_bounding;
}

static FN_DECIMAL SingleValueFractalRigidMulti3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	FN_DECIMAL sum = 1 - FastAbs(SingleValue3(fn, fn->perm[0], x, y, z));
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;
		z *= fn->lacunarity;

		amp *= fn->gain;
		sum -= (1 - FastAbs(SingleValue3(fn, fn->perm[i], x, y, z))) * amp;
	}

	return sum;
}

FN_DECIMAL fn_value_fractal3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	x *= fn->frequency;
	y *= fn->frequency;
	z *= fn->frequency;

	switch (fn->fractal_type)
	{
	case FN_FBM:
		return SingleValueFractalFBM3(fn, x, y, z);
	case FN_Billow:
		return SingleValueFractalBillow3(fn, x, y, z);
	case FN_RigidMulti:
		return SingleValueFractalRigidMulti3(fn, x, y, z);
	default:
		return 0;
	}
}


FN_DECIMAL fn_value3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	return SingleValue3(fn, 0, x * fn->frequency, y * fn->frequency, z * fn->frequency);
}

static FN_DECIMAL SingleValue2(FastNoise* fn, unsigned char offset, FN_DECIMAL x, FN_DECIMAL y)
{
	int x0 = FastFloor(x);
	int y0 = FastFloor(y);
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	FN_DECIMAL xs, ys;
	switch (fn->interp)
	{
	case FN_Linear:
		xs = x - (FN_DECIMAL)x0;
		ys = y - (FN_DECIMAL)y0;
		break;
	case FN_Hermite:
		xs = InterpHermiteFunc(x - (FN_DECIMAL)x0);
		ys = InterpHermiteFunc(y - (FN_DECIMAL)y0);
		break;
	case FN_Quintic:
		xs = InterpQuinticFunc(x - (FN_DECIMAL)x0);
		ys = InterpQuinticFunc(y - (FN_DECIMAL)y0);
		break;
	}

	FN_DECIMAL xf0 = Lerp(ValCoord2DFast(fn, offset, x0, y0), ValCoord2DFast(fn, offset, x1, y0), xs);
	FN_DECIMAL xf1 = Lerp(ValCoord2DFast(fn, offset, x0, y1), ValCoord2DFast(fn, offset, x1, y1), xs);

	return Lerp(xf0, xf1, ys);
}


FN_DECIMAL fn_value2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	return SingleValue2(fn, 0, x * fn->frequency, y * fn->frequency);
}

static FN_DECIMAL SingleValueFractalFBM2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL sum = SingleValue2(fn, fn->perm[0], x, y);
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;

		amp *= fn->gain;
		sum += SingleValue2(fn, fn->perm[i], x, y) * amp;
	}

	return sum * fn->fractal_bounding;
}

static FN_DECIMAL SingleValueFractalBillow2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL sum = FastAbs(SingleValue2(fn, fn->perm[0], x, y)) * 2 - 1;
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;
		amp *= fn->gain;
		sum += (FastAbs(SingleValue2(fn, fn->perm[i], x, y)) * 2 - 1) * amp;
	}

	return sum * fn->fractal_bounding;
}

static FN_DECIMAL SingleValueFractalRigidMulti2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL sum = 1 - FastAbs(SingleValue2(fn, fn->perm[0], x, y));
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;

		amp *= fn->gain;
		sum -= (1 - FastAbs(SingleValue2(fn, fn->perm[i], x, y))) * amp;
	}

	return sum;
}


FN_DECIMAL fn_value_fractal2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	x *= fn->frequency;
	y *= fn->frequency;

	switch (fn->fractal_type)
	{
	case FN_FBM:
		return SingleValueFractalFBM2(fn, x, y);
	case FN_Billow:
		return SingleValueFractalBillow2(fn, x, y);
	case FN_RigidMulti:
		return SingleValueFractalRigidMulti2(fn, x, y);
	default:
		return 0;
	}
}


// Perlin noise

static FN_DECIMAL SinglePerlin3(FastNoise* fn, unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	int x0 = FastFloor(x);
	int y0 = FastFloor(y);
	int z0 = FastFloor(z);
	int x1 = x0 + 1;
	int y1 = y0 + 1;
	int z1 = z0 + 1;

	FN_DECIMAL xs, ys, zs;
	switch (fn->interp)
	{
	case FN_Linear:
		xs = x - (FN_DECIMAL)x0;
		ys = y - (FN_DECIMAL)y0;
		zs = z - (FN_DECIMAL)z0;
		break;
	case FN_Hermite:
		xs = InterpHermiteFunc(x - (FN_DECIMAL)x0);
		ys = InterpHermiteFunc(y - (FN_DECIMAL)y0);
		zs = InterpHermiteFunc(z - (FN_DECIMAL)z0);
		break;
	case FN_Quintic:
		xs = InterpQuinticFunc(x - (FN_DECIMAL)x0);
		ys = InterpQuinticFunc(y - (FN_DECIMAL)y0);
		zs = InterpQuinticFunc(z - (FN_DECIMAL)z0);
		break;
	}

	FN_DECIMAL xd0 = x - (FN_DECIMAL)x0;
	FN_DECIMAL yd0 = y - (FN_DECIMAL)y0;
	FN_DECIMAL zd0 = z - (FN_DECIMAL)z0;
	FN_DECIMAL xd1 = xd0 - 1;
	FN_DECIMAL yd1 = yd0 - 1;
	FN_DECIMAL zd1 = zd0 - 1;

	FN_DECIMAL xf00 = Lerp(GradCoord3D(fn, offset, x0, y0, z0, xd0, yd0, zd0), GradCoord3D(fn, offset, x1, y0, z0, xd1, yd0, zd0), xs);
	FN_DECIMAL xf10 = Lerp(GradCoord3D(fn, offset, x0, y1, z0, xd0, yd1, zd0), GradCoord3D(fn, offset, x1, y1, z0, xd1, yd1, zd0), xs);
	FN_DECIMAL xf01 = Lerp(GradCoord3D(fn, offset, x0, y0, z1, xd0, yd0, zd1), GradCoord3D(fn, offset, x1, y0, z1, xd1, yd0, zd1), xs);
	FN_DECIMAL xf11 = Lerp(GradCoord3D(fn, offset, x0, y1, z1, xd0, yd1, zd1), GradCoord3D(fn, offset, x1, y1, z1, xd1, yd1, zd1), xs);

	FN_DECIMAL yf0 = Lerp(xf00, xf10, ys);
	FN_DECIMAL yf1 = Lerp(xf01, xf11, ys);

	return Lerp(yf0, yf1, zs);
}

FN_DECIMAL fn_perlin3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	return SinglePerlin3(fn, 0, x * fn->frequency, y * fn->frequency, z * fn->frequency);
}

static FN_DECIMAL SinglePerlinFractalFBM3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	FN_DECIMAL sum = SinglePerlin3(fn, fn->perm[0], x, y, z);
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;
		z *= fn->lacunarity;

		amp *= fn->gain;
		sum += SinglePerlin3(fn, fn->perm[i], x, y, z) * amp;
	}

	return sum * fn->fractal_bounding;
}

static FN_DECIMAL SinglePerlinFractalBillow3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	FN_DECIMAL sum = FastAbs(SinglePerlin3(fn, fn->perm[0], x, y, z)) * 2 - 1;
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;
		z *= fn->lacunarity;

		amp *= fn->gain;
		sum += (FastAbs(SinglePerlin3(fn, fn->perm[i], x, y, z)) * 2 - 1) * amp;
	}

	return sum * fn->fractal_bounding;
}

static FN_DECIMAL SinglePerlinFractalRigidMulti3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	FN_DECIMAL sum = 1 - FastAbs(SinglePerlin3(fn, fn->perm[0], x, y, z));
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;
		z *= fn->lacunarity;

		amp *= fn->gain;
		sum -= (1 - FastAbs(SinglePerlin3(fn, fn->perm[i], x, y, z))) * amp;
	}

	return sum;
}

FN_DECIMAL fn_perlin_fractal3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	x *= fn->frequency;
	y *= fn->frequency;
	z *= fn->frequency;

	switch (fn->fractal_type)
	{
	case FN_FBM:
		return SinglePerlinFractalFBM3(fn, x, y, z);
	case FN_Billow:
		return SinglePerlinFractalBillow3(fn, x, y, z);
	case FN_RigidMulti:
		return SinglePerlinFractalRigidMulti3(fn, x, y, z);
	default:
		return 0;
	}
}

static FN_DECIMAL SinglePerlin2(FastNoise* fn, unsigned char offset, FN_DECIMAL x, FN_DECIMAL y)
{
	int x0 = FastFloor(x);
	int y0 = FastFloor(y);
	int x1 = x0 + 1;
	int y1 = y0 + 1;

	FN_DECIMAL xs, ys;
	switch (fn->interp)
	{
	case FN_Linear:
		xs = x - (FN_DECIMAL)x0;
		ys = y - (FN_DECIMAL)y0;
		break;
	case FN_Hermite:
		xs = InterpHermiteFunc(x - (FN_DECIMAL)x0);
		ys = InterpHermiteFunc(y - (FN_DECIMAL)y0);
		break;
	case FN_Quintic:
		xs = InterpQuinticFunc(x - (FN_DECIMAL)x0);
		ys = InterpQuinticFunc(y - (FN_DECIMAL)y0);
		break;
	}

	FN_DECIMAL xd0 = x - (FN_DECIMAL)x0;
	FN_DECIMAL yd0 = y - (FN_DECIMAL)y0;
	FN_DECIMAL xd1 = xd0 - 1;
	FN_DECIMAL yd1 = yd0 - 1;

	FN_DECIMAL xf0 = Lerp(GradCoord2D(fn, offset, x0, y0, xd0, yd0), GradCoord2D(fn, offset, x1, y0, xd1, yd0), xs);
	FN_DECIMAL xf1 = Lerp(GradCoord2D(fn, offset, x0, y1, xd0, yd1), GradCoord2D(fn, offset, x1, y1, xd1, yd1), xs);

	return Lerp(xf0, xf1, ys);
}

FN_DECIMAL fn_perlin2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	return SinglePerlin2(fn, 0, x * fn->frequency, y * fn->frequency);
}

static FN_DECIMAL SinglePerlinFractalFBM2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL sum = SinglePerlin2(fn, fn->perm[0], x, y);
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;

		amp *= fn->gain;
		sum += SinglePerlin2(fn, fn->perm[i], x, y) * amp;
	}

	return sum * fn->fractal_bounding;
}

static FN_DECIMAL SinglePerlinFractalBillow2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL sum = FastAbs(SinglePerlin2(fn, fn->perm[0], x, y)) * 2 - 1;
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;

		amp *= fn->gain;
		sum += (FastAbs(SinglePerlin2(fn, fn->perm[i], x, y)) * 2 - 1) * amp;
	}

	return sum * fn->fractal_bounding;
}

static FN_DECIMAL SinglePerlinFractalRigidMulti2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL sum = 1 - FastAbs(SinglePerlin2(fn, fn->perm[0], x, y));
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;

		amp *= fn->gain;
		sum -= (1 - FastAbs(SinglePerlin2(fn, fn->perm[i], x, y))) * amp;
	}

	return sum;
}

FN_DECIMAL fn_perlin_fractal2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	x *= fn->frequency;
	y *= fn->frequency;

	switch (fn->fractal_type)
	{
	case FN_FBM:
		return SinglePerlinFractalFBM2(fn, x, y);
	case FN_Billow:
		return SinglePerlinFractalBillow2(fn, x, y);
	case FN_RigidMulti:
		return SinglePerlinFractalRigidMulti2(fn, x, y);
	default:
		return 0;
	}
}

static FN_DECIMAL F3 = 1 / (FN_DECIMAL)(3);
static FN_DECIMAL G3 = 1 / (FN_DECIMAL)(6);

static FN_DECIMAL SingleSimplex3(FastNoise* fn, unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	FN_DECIMAL t = (x + y + z) * F3;
	int i = FastFloor(x + t);
	int j = FastFloor(y + t);
	int k = FastFloor(z + t);

	t = (i + j + k) * G3;
	FN_DECIMAL X0 = i - t;
	FN_DECIMAL Y0 = j - t;
	FN_DECIMAL Z0 = k - t;

	FN_DECIMAL x0 = x - X0;
	FN_DECIMAL y0 = y - Y0;
	FN_DECIMAL z0 = z - Z0;

	int i1, j1, k1;
	int i2, j2, k2;

	if (x0 >= y0)
	{
		if (y0 >= z0)
		{
			i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
		}
		else if (x0 >= z0)
		{
			i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1;
		}
		else // x0 < z0
		{
			i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1;
		}
	}
	else // x0 < y0
	{
		if (y0 < z0)
		{
			i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1;
		}
		else if (x0 < z0)
		{
			i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1;
		}
		else // x0 >= z0
		{
			i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0;
		}
	}

	FN_DECIMAL x1 = x0 - i1 + G3;
	FN_DECIMAL y1 = y0 - j1 + G3;
	FN_DECIMAL z1 = z0 - k1 + G3;
	FN_DECIMAL x2 = x0 - i2 + 2 * G3;
	FN_DECIMAL y2 = y0 - j2 + 2 * G3;
	FN_DECIMAL z2 = z0 - k2 + 2 * G3;
	FN_DECIMAL x3 = x0 - 1 + 3 * G3;
	FN_DECIMAL y3 = y0 - 1 + 3 * G3;
	FN_DECIMAL z3 = z0 - 1 + 3 * G3;

	FN_DECIMAL n0, n1, n2, n3;

	t = (FN_DECIMAL)(0.6) - x0 * x0 - y0 * y0 - z0 * z0;
	if (t < 0) n0 = 0;
	else
	{
		t *= t;
		n0 = t * t*GradCoord3D(fn, offset, i, j, k, x0, y0, z0);
	}

	t = (FN_DECIMAL)(0.6) - x1 * x1 - y1 * y1 - z1 * z1;
	if (t < 0) n1 = 0;
	else
	{
		t *= t;
		n1 = t * t*GradCoord3D(fn, offset, i + i1, j + j1, k + k1, x1, y1, z1);
	}

	t = (FN_DECIMAL)(0.6) - x2 * x2 - y2 * y2 - z2 * z2;
	if (t < 0) n2 = 0;
	else
	{
		t *= t;
		n2 = t * t*GradCoord3D(fn, offset, i + i2, j + j2, k + k2, x2, y2, z2);
	}

	t = (FN_DECIMAL)(0.6) - x3 * x3 - y3 * y3 - z3 * z3;
	if (t < 0) n3 = 0;
	else
	{
		t *= t;
		n3 = t * t*GradCoord3D(fn, offset, i + 1, j + 1, k + 1, x3, y3, z3);
	}

	return 32 * (n0 + n1 + n2 + n3);
}

FN_DECIMAL fn_simplex3(FastNoise*fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z) 
{
	return SingleSimplex3(fn, 0, x * fn->frequency, y * fn->frequency, z * fn->frequency);
}

FN_DECIMAL SingleSimplexFractalFBM3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	FN_DECIMAL sum = SingleSimplex3(fn, fn->perm[0], x, y, z);
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;
		z *= fn->lacunarity;

		amp *= fn->gain;
		sum += SingleSimplex3(fn, fn->perm[i], x, y, z) * amp;
	}

	return sum * fn->fractal_bounding;
}

FN_DECIMAL SingleSimplexFractalBillow3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	FN_DECIMAL sum = FastAbs(SingleSimplex3(fn, fn->perm[0], x, y, z)) * 2 - 1;
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;
		z *= fn->lacunarity;

		amp *= fn->gain;
		sum += (FastAbs(SingleSimplex3(fn, fn->perm[i], x, y, z)) * 2 - 1) * amp;
	}

	return sum * fn->fractal_bounding;
}

FN_DECIMAL SingleSimplexFractalRigidMulti3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	FN_DECIMAL sum = 1 - FastAbs(SingleSimplex3(fn, fn->perm[0], x, y, z));
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;
		z *= fn->lacunarity;

		amp *= fn->gain;
		sum -= (1 - FastAbs(SingleSimplex3(fn, fn->perm[i], x, y, z))) * amp;
	}

	return sum;
}

FN_DECIMAL fn_simplex_fractal3(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	x *= fn->frequency;
	y *= fn->frequency;
	z *= fn->frequency;

	switch (fn->fractal_type)
	{
	case FN_FBM:
		return SingleSimplexFractalFBM3(fn, x, y, z);
	case FN_Billow:
		return SingleSimplexFractalBillow3(fn, x, y, z);
	case FN_RigidMulti:
		return SingleSimplexFractalRigidMulti3(fn, x, y, z);
	default:
		return 0;
	}
}

#define SQRT3 (FN_DECIMAL)(1.7320508075688772935274463415059)
#define F2 (FN_DECIMAL)(0.5) * (SQRT3 - (FN_DECIMAL)(1.0))
#define G2 ((FN_DECIMAL)(3.0) - SQRT3) / (FN_DECIMAL)(6.0)

static FN_DECIMAL SingleSimplex2(FastNoise* fn, unsigned char offset, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL t = (x + y) * F2;
	int i = FastFloor(x + t);
	int j = FastFloor(y + t);

	t = (i + j) * G2;
	FN_DECIMAL X0 = i - t;
	FN_DECIMAL Y0 = j - t;

	FN_DECIMAL x0 = x - X0;
	FN_DECIMAL y0 = y - Y0;

	int i1, j1;
	if (x0 > y0)
	{
		i1 = 1; j1 = 0;
	}
	else
	{
		i1 = 0; j1 = 1;
	}

	FN_DECIMAL x1 = x0 - (FN_DECIMAL)i1 + G2;
	FN_DECIMAL y1 = y0 - (FN_DECIMAL)j1 + G2;
	FN_DECIMAL x2 = x0 - 1 + 2 * G2;
	FN_DECIMAL y2 = y0 - 1 + 2 * G2;

	FN_DECIMAL n0, n1, n2;

	t = (FN_DECIMAL)(0.5) - x0 * x0 - y0 * y0;
	if (t < 0) n0 = 0;
	else
	{
		t *= t;
		n0 = t * t * GradCoord2D(fn, offset, i, j, x0, y0);
	}

	t = (FN_DECIMAL)(0.5) - x1 * x1 - y1 * y1;
	if (t < 0) n1 = 0;
	else
	{
		t *= t;
		n1 = t * t*GradCoord2D(fn, offset, i + i1, j + j1, x1, y1);
	}

	t = (FN_DECIMAL)(0.5) - x2 * x2 - y2 * y2;
	if (t < 0) n2 = 0;
	else
	{
		t *= t;
		n2 = t * t*GradCoord2D(fn, offset, i + 1, j + 1, x2, y2);
	}

	return 70 * (n0 + n1 + n2);
}

FN_DECIMAL fn_simplex2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	return SingleSimplex2(fn, 0, x * fn->frequency, y * fn->frequency);
}

FN_DECIMAL SingleSimplexFractalFBM2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL sum = SingleSimplex2(fn, fn->perm[0], x, y);
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;

		amp *= fn->gain;
		sum += SingleSimplex2(fn, fn->perm[i], x, y) * amp;
	}

	return sum * fn->fractal_bounding;
}

FN_DECIMAL SingleSimplexFractalBillow2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL sum = FastAbs(SingleSimplex2(fn, fn->perm[0], x, y)) * 2 - 1;
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;

		amp *= fn->gain;
		sum += (FastAbs(SingleSimplex2(fn, fn->perm[i], x, y)) * 2 - 1) * amp;
	}

	return sum * fn->fractal_bounding;
}

FN_DECIMAL SingleSimplexFractalRigidMulti2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL sum = 1 - FastAbs(SingleSimplex2(fn, fn->perm[0], x, y));
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;

		amp *= fn->gain;
		sum -= (1 - FastAbs(SingleSimplex2(fn, fn->perm[i], x, y))) * amp;
	}

	return sum;
}

FN_DECIMAL SingleSimplexFractalBlend2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	FN_DECIMAL sum = SingleSimplex2(fn,  fn->perm[0], x, y);
	FN_DECIMAL amp = 1;
	int i = 0;

	while (++i < fn->octaves)
	{
		x *= fn->lacunarity;
		y *= fn->lacunarity;

		amp *= fn->gain;
		sum *= SingleSimplex2(fn, fn->perm[i], x, y) * amp + 1;
	}

	return sum * fn->fractal_bounding;
}

FN_DECIMAL fn_simplex_fractal2(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y)
{
	x *= fn->frequency;
	y *= fn->frequency;

	switch (fn->fractal_type)
	{
	case FN_FBM:
		return SingleSimplexFractalFBM2(fn, x, y);
	case FN_Billow:
		return SingleSimplexFractalBillow2(fn, x, y);
	case FN_RigidMulti:
		return SingleSimplexFractalRigidMulti2(fn, x, y);
	default:
		return 0;
	}
}

#define SQRT5 (FN_DECIMAL)(2.2360679775)
#define F4 (FN_DECIMAL)((SQRT5 - 1) / 4)
#define G4 (FN_DECIMAL)((5 - SQRT5) / 20)

FN_DECIMAL SingleSimplex4(FastNoise* fn, unsigned char offset, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL w)
{
	FN_DECIMAL n0, n1, n2, n3, n4;
	FN_DECIMAL t = (x + y + z + w) * F4;
	int i = FastFloor(x + t);
	int j = FastFloor(y + t);
	int k = FastFloor(z + t);
	int l = FastFloor(w + t);
	t = (i + j + k + l) * G4;
	FN_DECIMAL X0 = i - t;
	FN_DECIMAL Y0 = j - t;
	FN_DECIMAL Z0 = k - t;
	FN_DECIMAL W0 = l - t;
	FN_DECIMAL x0 = x - X0;
	FN_DECIMAL y0 = y - Y0;
	FN_DECIMAL z0 = z - Z0;
	FN_DECIMAL w0 = w - W0;

	int rankx = 0;
	int ranky = 0;
	int rankz = 0;
	int rankw = 0;

	if (x0 > y0) rankx++; else ranky++;
	if (x0 > z0) rankx++; else rankz++;
	if (x0 > w0) rankx++; else rankw++;
	if (y0 > z0) ranky++; else rankz++;
	if (y0 > w0) ranky++; else rankw++;
	if (z0 > w0) rankz++; else rankw++;

	int i1 = rankx >= 3 ? 1 : 0;
	int j1 = ranky >= 3 ? 1 : 0;
	int k1 = rankz >= 3 ? 1 : 0;
	int l1 = rankw >= 3 ? 1 : 0;

	int i2 = rankx >= 2 ? 1 : 0;
	int j2 = ranky >= 2 ? 1 : 0;
	int k2 = rankz >= 2 ? 1 : 0;
	int l2 = rankw >= 2 ? 1 : 0;

	int i3 = rankx >= 1 ? 1 : 0;
	int j3 = ranky >= 1 ? 1 : 0;
	int k3 = rankz >= 1 ? 1 : 0;
	int l3 = rankw >= 1 ? 1 : 0;

	FN_DECIMAL x1 = x0 - i1 + G4;
	FN_DECIMAL y1 = y0 - j1 + G4;
	FN_DECIMAL z1 = z0 - k1 + G4;
	FN_DECIMAL w1 = w0 - l1 + G4;
	FN_DECIMAL x2 = x0 - i2 + 2 * G4;
	FN_DECIMAL y2 = y0 - j2 + 2 * G4;
	FN_DECIMAL z2 = z0 - k2 + 2 * G4;
	FN_DECIMAL w2 = w0 - l2 + 2 * G4;
	FN_DECIMAL x3 = x0 - i3 + 3 * G4;
	FN_DECIMAL y3 = y0 - j3 + 3 * G4;
	FN_DECIMAL z3 = z0 - k3 + 3 * G4;
	FN_DECIMAL w3 = w0 - l3 + 3 * G4;
	FN_DECIMAL x4 = x0 - 1 + 4 * G4;
	FN_DECIMAL y4 = y0 - 1 + 4 * G4;
	FN_DECIMAL z4 = z0 - 1 + 4 * G4;
	FN_DECIMAL w4 = w0 - 1 + 4 * G4;

	t = (FN_DECIMAL)(0.6) - x0 * x0 - y0 * y0 - z0 * z0 - w0 * w0;
	if (t < 0) n0 = 0;
	else {
		t *= t;
		n0 = t * t * GradCoord4D(fn, offset, i, j, k, l, x0, y0, z0, w0);
	}
	t = (FN_DECIMAL)(0.6) - x1 * x1 - y1 * y1 - z1 * z1 - w1 * w1;
	if (t < 0) n1 = 0;
	else {
		t *= t;
		n1 = t * t * GradCoord4D(fn, offset, i + i1, j + j1, k + k1, l + l1, x1, y1, z1, w1);
	}
	t = (FN_DECIMAL)(0.6) - x2 * x2 - y2 * y2 - z2 * z2 - w2 * w2;
	if (t < 0) n2 = 0;
	else {
		t *= t;
		n2 = t * t * GradCoord4D(fn, offset, i + i2, j + j2, k + k2, l + l2, x2, y2, z2, w2);
	}
	t = (FN_DECIMAL)(0.6) - x3 * x3 - y3 * y3 - z3 * z3 - w3 * w3;
	if (t < 0) n3 = 0;
	else {
		t *= t;
		n3 = t * t * GradCoord4D(fn, offset, i + i3, j + j3, k + k3, l + l3, x3, y3, z3, w3);
	}
	t = (FN_DECIMAL)(0.6) - x4 * x4 - y4 * y4 - z4 * z4 - w4 * w4;
	if (t < 0) n4 = 0;
	else {
		t *= t;
		n4 = t * t * GradCoord4D(fn, offset, i + 1, j + 1, k + 1, l + 1, x4, y4, z4, w4);
	}

	return 27 * (n0 + n1 + n2 + n3 + n4);
}

FN_DECIMAL fn_simplex4(FastNoise* fn, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z, FN_DECIMAL w)
{
	return SingleSimplex4(fn, 0, x * fn->frequency, y * fn->frequency, z * fn->frequency, w * fn->frequency);
}

void fn_set_crater_chance(FastNoise* fn, FN_DECIMAL chance)
{
	fn->crater_chance = chance;
}

void fn_set_crater_layers(FastNoise* fn, int layers)
{
	fn->crater_layers = layers;
}

// Maps a value that goes from 0 to 1, so it goes from min to max
static float map(float val, float min, float max)
{
	return val * (max - min) + min;
}

// Returns 1 if we are inside a crater
// TODO: Write this in FN_DECIMALS too? Performance?
// It should not really be needed as it's cell centered, but check it out!
static int crater_cell(FastNoise* fn, float* out_dist2, float* out_rad, float* out_radius, int seed,
						float cpx, float cpy, float cpz, int64_t cx, int64_t cy, int64_t cz)
{
	// We use this PCG for convenience, if it's slow we can use a simpler one
	rnd_pcg_t pcg;
	rnd_pcg_seed(&pcg, seed);
	float crater_sample = rnd_pcg_nextf(&pcg);
	if(crater_sample > fn->crater_chance)
	{
		return 0;
	}

	float radius = rnd_pcg_nextf(&pcg) * 0.5f;
	float pos_x = rnd_pcg_nextf(&pcg);
	float pos_y = rnd_pcg_nextf(&pcg);
	float pos_z = rnd_pcg_nextf(&pcg);

	float dist_x = cpx - pos_x;
	float dist_y = cpy - pos_y;
	float dist_z = cpz - pos_z;

	float dist2 = dist_x * dist_x + dist_y * dist_y + dist_z * dist_z;
	dist2 /= (radius * radius);
	if(dist2 > 1.0)
	{
		return 0;
	}

	*out_dist2 = (FN_DECIMAL)dist2;
	*out_radius = (FN_DECIMAL)radius;

	if (out_rad)
	{
		// Vector cp center of the evaluation position respect to crater
		// Vector base_dir is chosen at random for each crater, it's the cross product
		// of the crater position and a random vector (rarely will it aim towards (x,y,z) too)
		// in such a way that crater radials are aligned with the "planet surface"
		float rndx = rnd_pcg_nextf(&pcg);
		float rndy = rnd_pcg_nextf(&pcg);
		float rndz = rnd_pcg_nextf(&pcg);
		float cratx = pos_x + (float)cx;
		float craty = pos_y + (float)cy;
		float cratz = pos_z + (float)cz;
		float cratlength = sqrtf(cratx * cratx + craty * craty + cratz * cratz);
		cratx /= cratlength;
		craty /= cratlength;
		cratz /= cratlength;
		// bd = rnd x crat
		float bdx = rndy * cratz - rndz * craty;
		float bdy = rndz * cratx - rndx * cratz;
		float bdz = rndx * craty - rndy * cratx;
		// Project the evaluation position into the crat-center plane
		// we project dist
		float dist_along_normal = dist_x * cratx + dist_y * craty + dist_z * cratz;
		// eip = evaluation point in crat-center plane (bd is forcefully in the plane!)
		float eipx = dist_x - dist_along_normal * cratx;
		float eipy = dist_y - dist_along_normal * craty;
		float eipz = dist_z - dist_along_normal * cratz;

		float cplength = sqrtf(eipx * eipx + eipy * eipy + eipz * eipz);
		float bdlength = sqrtf(bdx * bdx + bdy * bdy + bdz * bdz);
		float dot = bdx * eipx + bdy * eipy + bdz * eipz;

		*out_rad = acosf(dot / (cplength * bdlength));
	}

	return 1;

}

FN_DECIMAL fn_crater3(FastNoise* fn, int calculate_rad, FN_DECIMAL x, FN_DECIMAL y, FN_DECIMAL z)
{
	float acc_dist2 = 1.0f;
	float acc_rad = 0.0f;
	float cur_min_radius = 999.9f;

	// The grid is always 1x1x1 so we use frequency to multiply x,y,z
	x *= fn->frequency;
	y *= fn->frequency;
	z *= fn->frequency;

	for(int i = 0; i < fn->crater_layers; i++)
	{
		// Shift the grid for more interesting generationbugged
		// TODO: This breaks crater centering for wathever reason...
		//x += 1.0f / (float)(fn->crater_layers);
		//y += 1.0f / (float)(fn->crater_layers);
		//z += 1.0f / (float)(fn->crater_layers);

		// Craters may only be as big as a cell (radius = 0.5), and thus can only affect HALF
		// of the neighboring cell. This greatly reduces the number of cells we have to check
		// Cell coordinates (int64 may be overkill, allows REALLY small craters if using double)
		int64_t cx = FastFloor(x);
		int64_t cy = FastFloor(y);
		int64_t cz = FastFloor(z);
		float cpx = (float) (x - (FN_DECIMAL) cx);
		float cpy = (float) (y - (FN_DECIMAL) cy);
		float cpz = (float) (z - (FN_DECIMAL) cz);
		// Sum contribution of neighbor cells and our own cell
		int64_t scx = cpx < 0.5f ? cx - 1 : cx;
		int64_t scy = cpy < 0.5f ? cy - 1 : cy;
		int64_t scz = cpz < 0.5f ? cz - 1 : cz;
		int64_t ecx = cpx < 0.5f ? cx : cx + 1;
		int64_t ecy = cpy < 0.5f ? cy : cy + 1;
		int64_t ecz = cpz < 0.5f ? cz : cz + 1;

		for (int64_t icx = scx; icx <= ecx; icx++)
		{
			for (int64_t icy = scy; icy <= ecy; icy++)
			{
				for (int64_t icz = scz; icz <= ecz; icz++)
				{
					// TODO: Improve the seeding, this implies a periodicity N of the noise
					// We could use a Z-curve for extra coolness!
					int N = 10000;
					int seed = (int) ((icz % N) * N * N + (icy % N) * N + (icx % N)) + fn->seed + i;
					int x_off = (int) (icx - cx);
					int y_off = (int) (icy - cy);
					int z_off = (int) (icz - cz);
					float c_cpx = cpx - (float) x_off;
					float c_cpy = cpy - (float) y_off;
					float c_cpz = cpz - (float) z_off;
					float radius;
					float dist2;
					float rad;
					float* rad_ptr = calculate_rad ? &rad : NULL;
					if (crater_cell(fn, &dist2, rad_ptr, &radius,
					 	seed, c_cpx, c_cpy, c_cpz, icx, icy, icz) == 1)
					{
						// Overlap mode: minimum of two
						cur_min_radius = radius;
						float minv = fminf(dist2, acc_dist2);
						if(minv == dist2)
						{
							acc_dist2 = dist2;
							acc_rad = rad;
						}
					}
				}
			}
		}
	}

	if(calculate_rad)
	{
		fn->crater_rad = acc_rad;
	}
	return acc_dist2;

}

FN_DECIMAL fn_crater3_get_rad(FastNoise* fn)
{
	return fn->crater_rad;
}

