#pragma once

#include <Eigen/Dense>
#include <vector>

struct Mesh
{
	struct Triangle
	{
		int v0, v1, v2;
		Triangle(int v0, int v1, int v2);
	};

	std::vector<Eigen::Vector3f> vertices;
	std::vector<Triangle>        faces;
};

Mesh LoadPly(const char * path);
