#pragma once

#include <vector>

#include <Eigen/Dense>

class LabValidator
{
public:

	LabValidator(size_t lookupTableSize);

	bool Check(Eigen::Vector3f lab);

private:

	inline int LookUp(float x) const;

	std::vector<int> t;
	float offset, factor;
};