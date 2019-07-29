#pragma once

#include "IModel.h"

#pragma warning( push )
#pragma warning(disable:4324) // structure was padded due to __declspec(align())

class BlendedModel final : public IModel
{
public:

	BlendedModel(const IModel & model1, const IModel & model2, float amount);

	inline __m256 operator [] (const Vector3f256 & p) const;

private:

	const IModel & model1;
	const IModel & model2;

	__m256 t;
};

#pragma warning( pop )