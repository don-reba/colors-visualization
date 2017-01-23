#include "BandValueMap.h"

BandValueMap::BandValueMap(float min, float max)
	: min(min), max(max)
{
}

float BandValueMap::operator[] (float x) const
{
	return (x >= min && x <= max) ? 1.0f : 0.0f;
}

__m256 BandValueMap::operator[] (__m256 x) const
{
	return _mm256_and_ps
		( _mm256_cmp_ps(x, _mm256_set1_ps(min), _CMP_GE_OS)
		, _mm256_cmp_ps(x, _mm256_set1_ps(max), _CMP_LE_OS)
		);
}
