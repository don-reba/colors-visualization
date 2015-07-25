#pragma once

#include <Eigen/Dense>

class Bezier
{
private:

	float ax;
	float bx;
	float cx;

	float ay;
	float by;
	float cy;

public:

	Bezier(Eigen::Vector2f p1, Eigen::Vector2f p2);

	float SampleCurveX(float t);

	float SampleCurveY(float t);

	float SampleCurveDerivativeX(float t);

	// Given an x value, find a parametric value it came from.
	float SolveCurveX(float x, float epsilon);

	// Solve for x ± epsilon.
	float Solve(float x, float epsilon);
};
