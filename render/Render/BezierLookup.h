#pragma once

#include <vector>

#include <Eigen/Dense>

class BezierLookup
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

	size_t size() const;

private:

	float min;
	float max;

	std::vector<float> t;
};
