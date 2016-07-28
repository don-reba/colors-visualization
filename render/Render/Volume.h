#pragma once

#include "IModel.h"

#include <vector>

#include <Eigen/Dense>

class Volume : public IModel
{
public:

	enum Postprocess
	{
		PostprocessLog,
		PostprocessNone
	};

public:

	int Nx;
	int Ny;
	int Nz;

	std::vector<float> Values;

private:

	float xFactor;
	float yFactor;
	float zFactor;

public:

	Volume(const char * path, Postprocess method);

	float operator [] (const Eigen::Vector3f & lab) const;
};
