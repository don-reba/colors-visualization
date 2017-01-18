#include "MappedModel.h"

using namespace Eigen;

MappedModel::MappedModel(const IModel & model, const IValueMap & valueMap)
	: model(model), valueMap(valueMap)
{
}

float MappedModel::operator [] (const Vector3f & p) const
{
	return valueMap[model[p]];
}
__m256 MappedModel::operator [] (const Vector3f256 & p) const
{
	return valueMap[model[p]];
}
