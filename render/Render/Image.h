#pragma once

#include <Eigen/Dense>

void SaveBuffer
	( const char            * path
	, size_t                  width
	, size_t                  height
	, const Eigen::Vector4f * buffer
	, Eigen::Vector3f         bgColor
	);