#pragma once

#include "IModel.h"
#include "IValueMap.h"

class MappedModel final : public IModel
{
public:

	MappedModel(const IModel & model, const IValueMap & valueMap);

	inline __m256 operator [] (const Vector3f256 & p) const;

private:

	const IModel    & model;
	const IValueMap & valueMap;
};