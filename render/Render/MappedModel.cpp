#include "MappedModel.h"

MappedModel::MappedModel(const IModel & model, const IValueMap & valueMap)
	: model(model), valueMap(valueMap)
{
}

__m256 MappedModel::operator [] (const Vector3f256 & p) const
{
	return valueMap[model[p]];
}
