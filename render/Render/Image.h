#pragma once

#include <Eigen/Dense>

void SaveBuffer
	( const char            * path
	, unsigned int            width
	, unsigned int            height
	, const Eigen::Vector4f * buffer
	, Eigen::Vector3f         bgColor
	);