#pragma once

#include "Image.h"
#include "Mesh.h"
#include "Volume.h"

#include <Eigen/Dense>

void RenderMesh
	( const Eigen::Matrix4f & world
	, const Eigen::Matrix3f & rayCast
	,       size_t            w
	,       size_t            h
	,       Eigen::Vector4f * buffer
	, const Mesh            & mesh
	, const Volume          & volume
	);