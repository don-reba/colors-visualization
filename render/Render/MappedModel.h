#pragma once

#include "IModel.h"
#include "IValueMap.h"

class MappedModel : public IModel
{
public:

	MappedModel(const IModel & model, const IValueMap & valueMap);

	inline float operator [] (const Eigen::Vector3f & lab) const;

	inline __m256 operator [] (const Vector3f256 & lab) const;

private:

	const IModel    & model;
	const IValueMap & valueMap;
};