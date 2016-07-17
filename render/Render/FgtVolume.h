#pragma once

#include "IModel.h"

#include <vector>

#include <Eigen\Dense>

class FgtVolume : public IModel
{
public:

public:
	float Sigma;
	int   Alpha;
	int   PD;
	float Side;

	int N;

	Eigen::Vector3f DomainMin, DomainMax;
	Eigen::Vector3i Count;

	std::vector<float> Values;

public:
	FgtVolume(const char * path, int n);

	float operator [] (const Eigen::Vector3f & lab) const;
};