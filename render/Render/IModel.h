#pragma once

#include <immintrin.h> // AVX

struct Vector3f256 final
{
	__m256 x;
	__m256 y;
	__m256 z;

	static Vector3f256 Zero()
	{
		Vector3f256 v;
		v.x = _mm256_setzero_ps();
		v.y = _mm256_setzero_ps();
		v.z = _mm256_setzero_ps();
		return v;
	}
};

struct IModel
{
	virtual ~IModel() = default;

	virtual __m256 operator [] (const Vector3f256 & lab) const = 0;
};
