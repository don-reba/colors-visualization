#pragma once

#include "Antialias.h"
#include "IValueMap.h"
#include "IModel.h"
#include "Image.h"
#include "Mesh.h"
#include "Profiler.h"
#include "RateIndicator.h"
#include "Resolution.h"

#include <Eigen/Dense>

void RenderMesh
	( const Eigen::Matrix4f & camera
	, const Eigen::Matrix3f & rayCast
	,       Resolution        res
	,       Eigen::Vector4f * buffer
	, const Mesh            & mesh
	, const IModel          & model
	, const AAMask          & aamask
	,       float             stepLength
	,       Profiler        & profiler
	,       RateIndicator   & rateIndicator
	);