#include "BezierLookup.h"

#include "Bezier.h"

#include <stdexcept>

using namespace Eigen;

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
{
	float epsilon(0.5f / size);
	Bezier b(p1, p2);
	for (size_t i(0); i != size; ++i)
		t[i] = b.Solve(1.0f * i / size, epsilon);
	t[size] = 1.0f;
}

float BezierLookup::operator[] (float x) const
{
	if (x < min) x = min;
	if (x > max) x = max;
	x = (x - min) / (max - min);
	return t[static_cast<size_t>(x * (t.size() - 1))];
}

size_t BezierLookup::size() const
{
	return t.size() - 1;
}