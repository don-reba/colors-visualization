#pragma once

#include "BandValueMap.h"
#include "MappedModel.h"
#include "ModelCache.h"
#include "PiecewiseLinearValueMap.h"
#include "Script.h"

#include <boost/align/aligned_delete.hpp>

#include <Eigen/Dense>

#include <memory>
#include <string>

class Animation final
{
private:

	template <typename T>
	using aligned_unique_ptr = std::unique_ptr<T, boost::alignment::aligned_delete>;

public:

	Animation(float animationSeconds, float rotationSeconds, ModelCache & modelCache, const char * modelPath);

	const Eigen::Matrix4f & GetCamera() const;
	const IModel          & GetModel()  const;

	void SetTime(float time);

private:

	void SetCamera(float time);

	void SetModel(float time);

private:

	const float animationSeconds;
	const float rotationSeconds;

	Eigen::Matrix4f camera;

	const IModel * baseModel;

	PiecewiseLinearValueMap bandMap;

	aligned_unique_ptr<BandValueMap> valueMap;
	aligned_unique_ptr<MappedModel>  model;
};