#include "PiecewiseLinearValueMap.h"

#include <algorithm>
#include <stdexcept>

PiecewiseLinearValueMap::PiecewiseLinearValueMap(Points points) noexcept
	: points(std::move(points))
{
}

float PiecewiseLinearValueMap::operator[] (float x) const
{
	const auto lb = std::lower_bound(points.begin(), points.end(), std::tuple(x, 0.0f));
	if (lb == points.end())
		throw std::invalid_argument("Input outside of map.");
	if (lb == points.begin())
		return std::get<1>(*lb);

	float x1, y1, x2, y2;
	std::tie(x1, y1) = *(lb - 1);
	std::tie(x2, y2) = *(lb - 0);
	return y1 + (x - x1) * (y2 - y1) / (x2 - x1);
}