#pragma once

#include <Eigen/Dense>

#include <vector>

struct LabToRgbLookup
{
	LabToRgbLookup(size_t size);

	inline float GetValue(float x) const;

private:
	std::vector<float> t;
	float offset, factor;
};

Eigen::Vector3f LabToRgb(Eigen::Vector3f lab);
Eigen::Vector3f LabToRgb(Eigen::Vector3f lab, const LabToRgbLookup & lookup);

Eigen::Vector3f RgbToLab(Eigen::Vector3f rgb);
