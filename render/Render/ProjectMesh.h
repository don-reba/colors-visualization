#pragma once

#include "Image.h"
#include "Mesh.h"
#include "Resolution.h"

#include <Eigen/Dense>

void ProjectMesh
	( const Eigen::Matrix4f & world
	, const Eigen::Matrix4f & projection
	,       Resolution        res
	,       Eigen::Vector4f * buffer
	, const Mesh            & mesh
	);