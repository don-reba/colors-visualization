#include "BezierLookup.h"

#include "Bezier.h"

#include <stdexcept>

using namespace Eigen;
using namespace std;

BezierLookup::BezierLookup
	( Vector2f p1
	, Vector2f p2
	, size_t   size
	, float    min
	, float    max
	)
	: t(size + 1)
	, min(min)
	, max(max)
	, factor((float)size / (max - min))
{
	if (min >= max)
	throw runtime_error("BezierDirect: min >= max");

	float epsilon(0.5f / size);
	Bezier b(p1, p2);
	for (size_t i(0); i != size; ++i)
		t[i] = b.Solve(1.0f * i / size, epsilon);
	t[size] = 1.0f;
}

float BezierLookup::operator[] (float x) const
{
	if (x < min) return 0.0f;
	if (x > max) return 1.0f;
	x = factor * (x - min);
	return t[static_cast<size_t>(x)];
}

__m256 BezierLookup::operator[] (__m256 x) const
{
	__declspec(align(32)) float values[8];
	_mm256_store_ps(values, x);

	__declspec(align(32)) float result[8];
	for (size_t i = 0; i != 8; ++i)
		result[i] = (*this)[values[i]];

	return _mm256_load_ps(result);
}
