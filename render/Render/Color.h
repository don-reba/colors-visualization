#pragma once

#include <Eigen/Dense>

#include <vector>

class LabToRgbLookup
{
public:

	LabToRgbLookup(size_t size);

	inline int operator[] (float x) const;

private:

	std::vector<int> t;
	float offset, factor;
};

Eigen::Vector3f LabToRgb(Eigen::Vector3f lab);
Eigen::Vector3f RgbToLab(Eigen::Vector3f rgb);

bool IsValidLab(Eigen::Vector3f lab, const LabToRgbLookup & lookup);