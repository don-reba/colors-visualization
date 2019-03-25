#pragma once

#include "IModel.h"
#include "IValueMap.h"

#include <boost/align/aligned_delete.hpp>

#include <Eigen/Dense>

#include <memory>
#include <string>

class Animation
{
private:

	template <typename T>
	using aligned_unique_ptr = std::unique_ptr<T, boost::alignment::aligned_delete>;

	const std::string projectRoot;

	const float duration;

	Eigen::Matrix4f camera;

	const IModel & baseModel;

	aligned_unique_ptr<IValueMap> valueMap;
	aligned_unique_ptr<IModel>    model;

public:

	Animation(float duration, const IModel & model);

	const Eigen::Matrix4f & GetCamera() const;
	const IModel          & GetModel()  const;

	void SetTime(float time);

private:

	void SetCamera(float time);

	void SetModel(float time);
};