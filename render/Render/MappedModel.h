#pragma once

#include "IModel.h"
#include "IValueMap.h"

class MappedModel : public IModel
{
public:

	MappedModel(const IModel & model, const IValueMap & valueMap);

	inline float operator [] (const Eigen::Vector3f & p) const;

	inline __m256 operator [] (const Vector3f256 & p) const;

private:

	const IModel    & model;
	const IValueMap & valueMap;
};