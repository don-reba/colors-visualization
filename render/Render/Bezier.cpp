// Borrowed from WebKit.

#include "Bezier.h"
#include <math.h>

using namespace Eigen;

Bezier::Bezier(Vector2f p1, Vector2f p2)
{
	// Calculate the polynomial coefficients, implicit first and last control points are (0,0) and (1,1).
	cx = 3.0f * p1.x();
	bx = 3.0f * (p2.x() - p1.x()) - cx;
	ax = 1.0f - cx - bx;

	cy = 3.0f * p1.y();
	by = 3.0f * (p2.y() - p1.y()) - cy;
	ay = 1.0f - cy - by;
}

float Bezier::SampleCurveX(float t) const
{
	// `ax t^3 + bx t^2 + cx t' expanded using Horner's rule.
	return ((ax * t + bx) * t + cx) * t;
}

float Bezier::SampleCurveY(float t) const
{
	return ((ay * t + by) * t + cy) * t;
}

float Bezier::SampleCurveDerivativeX(float t) const
{
	return (3.0f * ax * t + 2.0f * bx) * t + cx;
}

// Given an x value, find a parametric value it came from.
float Bezier::SolveCurveX(float x, float epsilon) const
{
	// try a few Newton's method iterations
	float t = x;
	for (size_t i = 0; i < 8; i++) {
		float x2 = SampleCurveX(t) - x;
		if (fabs(x2) < epsilon)
			return t;
		float d2 = SampleCurveDerivativeX(t);
		if (fabs(d2) < 1e-6)
			break;
		t = t - x2 / d2;
	}

	return t;
}

float Bezier::Solve(float x, float epsilon) const
{
	return SampleCurveY(SolveCurveX(x, epsilon));
}