#pragma once

#include <immintrin.h> // AVX

struct Vector3f256
{
	__m256 x;
	__m256 y;
	__m256 z;
};

struct IModel
{
	virtual ~IModel() {}

	virtual __m256 operator [] (const Vector3f256 & lab) const = 0;
};
