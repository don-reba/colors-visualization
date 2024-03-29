#pragma once

#include <Eigen/Dense>

#include "Resolution.h"

void AddNoise
	( Resolution        res
	, Eigen::Vector4f * buffer
	, float             amount
	);

void SaveProjectionBuffer
	( const char            * path
	, Resolution              res
	, const Eigen::Vector4f * buffer
	);

void SaveBuffer
	( const char            * path
	, Resolution              res
	, const Eigen::Vector4f * buffer
	, Eigen::Vector3f         bgColor
	);