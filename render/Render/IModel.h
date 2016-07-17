#pragma once

#include <Eigen\Dense>

struct IModel
{
	virtual float operator [] (const Eigen::Vector3f & lab) const = 0;
};
