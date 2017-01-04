#pragma once

#include <immintrin.h> // AVX

struct IBezier
{
	virtual float operator[] (float x) const = 0;

	virtual __m256 operator[] (__m256 x) const = 0;
};
