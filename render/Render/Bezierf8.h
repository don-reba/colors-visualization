#pragma once

#include <Eigen/Dense>

#include <boost/align/aligned_allocator.hpp>

class Bezierf8
{
private:

	__m256 ax;
	__m256 bx;
	__m256 cx;

	__m256 ay;
	__m256 by;
	__m256 cy;

public:

	Bezierf8(Eigen::Vector2f p1, Eigen::Vector2f p2);

	__m256 SampleCurveX(__m256 t) const;

	__m256 SampleCurveY(__m256 t) const;

	__m256 SampleCurveDerivativeX(__m256 t) const;

	// Given an x value, find a parametric value it came from.
	__m256 SolveCurveX(__m256 x, __m256 epsilon) const;

	// Solve for x ± epsilon.
	__m256 Solve(__m256, __m256 epsilon) const;
};
