#pragma once

#include "IValueMap.h"
#include <Eigen/Dense>

#include <tuple>

class PiecewiseLinearValueMap final
{
public:

	using Point  = std::tuple<float, float>;
	using Points = std::vector<Point>;

public:

	PiecewiseLinearValueMap(Points points);

	float operator[] (float x) const;

private:

	Points points;
};
