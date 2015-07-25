#pragma once

#include <vector>

#include <Eigen/Dense>

class BezierLookup
{
public:

	// Solve for x ± epsilon.
	BezierLookup
		( Eigen::Vector2f p1
		, Eigen::Vector2f p2
		, float           epsilon
		, size_t          size
		);

	// x has to be in [0, 1]
	float operator[] (float x) const;

	size_t size() const;

private:

	std::vector<float> t;
};
