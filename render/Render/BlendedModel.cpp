#include "BlendedModel.h"

BlendedModel::BlendedModel(const IModel & model1, const IModel & model2, float amount)
	: model1(model1), model2(model2), t(_mm256_set1_ps(amount)) {}

__m256 BlendedModel::operator [] (const Vector3f256 & p) const
{
	// return x + t * (y - x)
	const __m256 x = model1[p];
	const __m256 y = model2[p];
	return _mm256_fmadd_ps(t, _mm256_sub_ps(y, x), x);
}
