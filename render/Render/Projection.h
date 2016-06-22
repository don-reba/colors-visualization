#pragma once

#include <Eigen/Dense>

Eigen::Matrix4f LookAt
	( const Eigen::Vector3f & eye
	, const Eigen::Vector3f & at
	, const Eigen::Vector3f & up
	);

Eigen::Matrix4f Perspective(float focalDistance);

Eigen::Matrix3f RayCast(float width, float height, float focalDistance);
