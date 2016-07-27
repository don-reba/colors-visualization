#pragma once

#include "IModel.h"

#include <vector>

#include <boost/align/aligned_allocator.hpp>

#include <Eigen\Dense>

class FgtVolume : public IModel
{
private:

	using aligned_vector_f32
		= std::vector<float, boost::alignment::aligned_allocator<float, 32>>;

public:

	struct Neighbors
	{
		aligned_vector_f32 vOffsets;
		std::vector<int>   iOffsets;
	};

public:

	float Sigma;
	int   Alpha;
	int   PD;
	float Side;

	Eigen::Vector3f DomainMin, DomainMax;
	Eigen::Vector3i Count;

	aligned_vector_f32 Coefficients;

private:

	Neighbors neighbors;

public:

	FgtVolume(const char * path);

	float operator [] (const Eigen::Vector3f & lab) const;

private:

	Neighbors PrecomputeNeighbors(int r) const;

public:

	Eigen::Vector3f ClusterOrigin(const Eigen::Vector3f & p) const;

	int CellIndex(const Eigen::Vector3f & p) const;
};