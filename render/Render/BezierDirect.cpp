#include "BezierDirect.h"

#include <stdexcept>

using namespace Eigen;
using namespace std;

BezierDirect::BezierDirect(Vector2f p1, Vector2f p2, float min, float max, float epsilon)
	: b(p1, p2), min(min), max(max), epsilon(epsilon)
{
	if (min >= max)
		throw runtime_error("BezierDirect: min >= max");
	if (epsilon <= 0.0)
		throw runtime_error("BezierDirect: epsilon <= 0");
}

float BezierDirect::operator[] (float x) const
{
	if (x < min) return 0.0f;
	if (x > max) return 1.0f;
	x = (x - min) / (max - min);
	return b.Solve(x, epsilon);
}

__m256 BezierDirect::operator[] (__m256 x) const
{
	__m256 min = _mm256_set1_ps(this->min);
	__m256 max = _mm256_set1_ps(this->max);

	x = _mm256_max_ps(x, min);
	x = _mm256_min_ps(x, max);
	x = _mm256_div_ps(_mm256_sub_ps(x, min), _mm256_sub_ps(max, min));

	__declspec(align(32)) float values[8];
	_mm256_store_ps(values, x);

	__declspec(align(32)) float result[8];
	for (size_t i = 0; i != 8; ++i)
		result[i] = b.Solve(values[i], epsilon);

	return _mm256_load_ps(result);
}