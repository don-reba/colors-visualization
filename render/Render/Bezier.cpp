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

float Bezier::SampleCurveX(float t)
{
	// `ax t^3 + bx t^2 + cx t' expanded using Horner's rule.
	return ((ax * t + bx) * t + cx) * t;
}

float Bezier::SampleCurveY(float t)
{
	return ((ay * t + by) * t + cy) * t;
}

float Bezier::SampleCurveDerivativeX(float t)
{
	return (3.0f * ax * t + 2.0f * bx) * t + cx;
}

// Given an x value, find a parametric value it came from.
float Bezier::SolveCurveX(float x, float epsilon)
{
	float t0;
	float t1;
	float t2;
	float x2;
	float d2;
	int i;

	// First try a few iterations of Newton's method -- normally very fast.
	for (t2 = x, i = 0; i < 8; i++) {
		x2 = SampleCurveX(t2) - x;
		if (fabs(x2) < epsilon)
			return t2;
		d2 = SampleCurveDerivativeX(t2);
		if (fabs(d2) < 1e-6)
			break;
		t2 = t2 - x2 / d2;
	}

	// Fall back to the bisection method for reliability.
	t0 = 0.0f;
	t1 = 1.0f;
	t2 = x;

	if (t2 < t0)
		return t0;
	if (t2 > t1)
		return t1;

	while (t0 < t1) {
		x2 = SampleCurveX(t2);
		if (fabs(x2 - x) < epsilon)
			return t2;
		if (x > x2)
			t0 = t2;
		else
			t1 = t2;
		t2 = (t1 - t0) * 0.5f + t0;
	}

	// Failure.
	return t2;
}

float Bezier::Solve(float x, float epsilon)
{
	return SampleCurveY(SolveCurveX(x, epsilon));
}