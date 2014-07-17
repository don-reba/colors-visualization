#pragma once

#include "Image.h"
#include "Mesh.h"

#include <Eigen/Dense>

void ProjectMesh
	( const Eigen::Matrix4f & world
	, const Eigen::Matrix4f & projection
	,       size_t            w
	,       size_t            h
	,       Pixel           * buffer
	, const Mesh            & mesh
	);