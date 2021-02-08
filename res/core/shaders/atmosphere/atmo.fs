#version 430 core

#include <quality_defines>
#ifdef _ATMO_LOW_END
#include <core:shaders/atmosphere/lowend/atmo.fs>
#else
#include <core:shaders/atmosphere/highend/atmo.fs>
#endif

