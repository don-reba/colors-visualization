#pragma once

#include "IBezier.h"
#include "Bezier.h"
#include <Eigen/Dense>

class BezierDirect : public IBezier
{
public:

	BezierDirect
		( Eigen::Vector2f p1
		, Eigen::Vector2f p2
		, float           min
		, float           max
		, float           epsilon
		);

	float operator[] (float x) const;

	__m256 operator[] (__m256 x) const;

private:

	Bezier b;
	float  min;
	float  max;
	float  epsilon;
};
