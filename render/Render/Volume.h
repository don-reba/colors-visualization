#pragma once

#include <vector>

#include <Eigen/Dense>

class Volume
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

	Volume(size_t nx, size_t ny, size_t nz);
	Volume(Volume && other);

	float operator [] (const Eigen::Vector3f & lab) const;

	static Volume MakeTest();
	static Volume Load(const char * path, Postprocess method);
};
