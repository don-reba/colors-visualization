#pragma once

#include "BlendedModel.h"
#include "BezierValueMap.h"
#include "MappedModel.h"
#include "ModelCache.h"
#include "Path.h"

#include <boost/align/aligned_delete.hpp>

#include <Eigen/Dense>

#include <memory>
#include <string>

class Animation final
{
public:

	Animation(float duration, ModelCache & modelCache, const char * modelPath);

	const Eigen::Matrix4f & GetCamera() const;
	const IModel          & GetModel()  const;

	void SetTime(float time);

private:

	void SetCamera(float time);
	void SetModel(float time);

private:

	const Path modelPath;

	const float duration;

	Eigen::Matrix4f camera;

	aligned_unique_ptr<BezierValueMap> valueMap;

	ModelCache & modelCache;

	IModel * startModel = nullptr;
	IModel * endModel   = nullptr;

	aligned_unique_ptr<BlendedModel> blendedModel;
	aligned_unique_ptr<MappedModel>  mappedModel;

};