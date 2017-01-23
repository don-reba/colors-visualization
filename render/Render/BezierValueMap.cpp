#include "BezierValueMap.h"

#include <stdexcept>

using namespace Eigen;
using namespace std;

BezierValueMap::BezierValueMap(Vector2f p1, Vector2f p2, float min, float max, float epsilon)
	: bf8(p1, p2), min(min), max(max), epsilon(epsilon)
	, factor(1.0f / (max - min)), offset(min / (min - max))
{
	if (min >= max)
		throw runtime_error("BezierValueMap: min >= max");
	if (epsilon <= 0.0)
		throw runtime_error("BezierValueMap: epsilon <= 0");
}

__m256 BezierValueMap::operator[] (__m256 x) const
{
	__m256 minf8 = _mm256_set1_ps(min);
	__m256 maxf8 = _mm256_set1_ps(max);

	__m256 offsetf8 = _mm256_set1_ps(offset);
	__m256 factorf8 = _mm256_set1_ps(factor);

	x = _mm256_max_ps(x, minf8);
	x = _mm256_min_ps(x, maxf8);
	x = _mm256_fmadd_ps(x, factorf8, offsetf8);

	return bf8.Solve(x, _mm256_set1_ps(epsilon));
}