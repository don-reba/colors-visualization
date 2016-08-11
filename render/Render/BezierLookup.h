#pragma once

#include "IBezier.h"

#include <vector>

#include <Eigen/Dense>

class BezierLookup : public IBezier
{
public:

	BezierLookup
		( Eigen::Vector2f p1
		, Eigen::Vector2f p2
		, size_t          size
		, float           min
		, float           max
		);

	float operator[] (float x) const;

private:

	float min;
	float max;
	float factor;

	std::vector<float> t;
};
