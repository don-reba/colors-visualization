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
	// computes:
	// Bezier((clamp(x, min, max) - min) / (max - min))

	x = _mm256_max_ps(x, _mm256_set1_ps(min));
	x = _mm256_min_ps(x, _mm256_set1_ps(max));
	x = _mm256_fmadd_ps(x, _mm256_set1_ps(factor), _mm256_set1_ps(offset));

	return bf8.Solve(x, _mm256_set1_ps(epsilon));
}