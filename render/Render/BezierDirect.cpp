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