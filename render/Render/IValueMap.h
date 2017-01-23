#pragma once

#include <immintrin.h> // AVX

struct IValueMap
{
	virtual ~IValueMap() {}

	virtual __m256 operator[] (__m256 x) const = 0;
};
