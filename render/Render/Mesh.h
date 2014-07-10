#pragma once

#include <Eigen/Dense>
#include <vector>

struct Mesh
{
	std::vector<Eigen::Vector3f> vertices;
	std::vector<int>             triList;
};

Mesh LoadPly(const char * path);
