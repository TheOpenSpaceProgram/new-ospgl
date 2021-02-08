#include <quality_defines>
#ifdef _ATMO_LOW_END
#include <core:shaders/atmosphere/lowend/atmo.fsi>
#else
#include <core:shaders/atmosphere/highend/atmo.fsi>
#endif
