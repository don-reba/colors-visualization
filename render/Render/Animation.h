#pragma once

#include <Eigen/Dense>

class RotationAnimation
{
private:

	Eigen::Vector3f eye;

public:

	RotationAnimation(Eigen::Vector3f eye);

	Eigen::Vector3f Eye(size_t step, size_t count) const;
};