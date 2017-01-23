#pragma once

#include "IValueMap.h"
#include "Bezier.h"
#include "Bezierf8.h"
#include <Eigen/Dense>

class BezierValueMap : public IValueMap
{
public:

	BezierValueMap
		( Eigen::Vector2f p1
		, Eigen::Vector2f p2
		, float           min
		, float           max
		, float           epsilon
		);

	__m256 operator[] (__m256 x) const;

private:

	Bezierf8 bf8;
	float    min;
	float    max;
	float    epsilon;
	float    factor;
	float    offset;
};
