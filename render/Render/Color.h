#pragma once

#include <Eigen/Dense>

#include <vector>

struct LabToRgbLookup
{
	LabToRgbLookup(size_t size);

	inline int GetValue(float x) const;

	std::vector<int> t;
	float offset, factor;
};

Eigen::Vector3f LabToRgb(Eigen::Vector3f lab);
Eigen::Vector3f RgbToLab(Eigen::Vector3f rgb);

bool IsValidLab(Eigen::Vector3f lab, const LabToRgbLookup & lookup);