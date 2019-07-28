#pragma once

#include "BezierValueMap.h"
#include "MappedModel.h"

#include <boost/align/aligned_delete.hpp>

#include <Eigen/Dense>

#include <memory>
#include <string>

class Animation final
{
private:

	template <typename T>
	using aligned_unique_ptr = std::unique_ptr<T, boost::alignment::aligned_delete>;

	const std::string projectRoot;

	const float duration;

	Eigen::Matrix4f camera;

	const IModel & baseModel;

	aligned_unique_ptr<BezierValueMap> valueMap;
	aligned_unique_ptr<MappedModel>    model;

public:

	Animation(float duration, const IModel & model);

	const Eigen::Matrix4f & GetCamera() const;
	const IModel          & GetModel()  const;

	void SetTime(float time);

private:

	void SetCamera(float time);
};