#pragma once

#include "IModel.h"

#include <vector>

#include <boost/align/aligned_allocator.hpp>

#include <Eigen/Dense>

class FgtVolume : public IModel
{
private:

	using aligned_vector_f32
		= std::vector<float, boost::alignment::aligned_allocator<float, 32>>;

public:

	float Sigma;
	int   Alpha;
	int   PD;
	float Side;

	Eigen::Vector3f DomainMin, DomainMax;
	Eigen::Vector3i Count;

	aligned_vector_f32 Coefficients;

private:

	float normalizationFactor;

public:

	FgtVolume(const char * path);

	float operator [] (const Eigen::Vector3f & p) const;

	__m256 operator [] (const Vector3f256 & p) const;
};