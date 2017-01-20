#pragma once

#include "IModel.h"
#include "IValueMap.h"

#include <Eigen/Dense>

#include <memory>
#include <string>

class Animation
{
private:

	const std::string projectRoot;

	const float duration;

	Eigen::Matrix4f camera;

	std::unique_ptr<IModel>    baseModel;
	std::unique_ptr<IValueMap> valueMap;
	std::unique_ptr<IModel>    model;

public:

	Animation(float duration, const char * projectRoot);

	const Eigen::Matrix4f & GetCamera() const;
	const IModel          & GetModel()  const;

	void SetTime(float time);

private:

	void SetCamera(float time);

	void SetModel(float time);
};