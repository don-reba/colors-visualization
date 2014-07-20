#pragma once

#include <Eigen/Dense>

struct Pixel
{
	float Alpha, L, A, B;

	Pixel() : Alpha(0.0f), L(0.0f), A(0.0f), B(0.0f)
	{
	}

	inline void Set(float alpha, const Eigen::Vector3f & lab)
	{
		Alpha = alpha;
		L = lab.x();
		A = lab.y();
		B = lab.z();
	}
};

void SaveBuffer(const char * path, size_t width, size_t height, const Pixel * buffer);