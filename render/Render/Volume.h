#pragma once

#include "IModel.h"

#include <vector>
#include <cstdint>

#include <Eigen/Dense>

class Volume : public IModel
{
private:

	int32_t nx;
	int32_t ny;
	int32_t nz;

	Eigen::Vector3f offset;
	float           factor;

	std::vector<float> values;

public:

	Volume(const char * path);

	float operator [] (const Eigen::Vector3f & lab) const;
};
