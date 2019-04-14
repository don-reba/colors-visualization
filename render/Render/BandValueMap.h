#pragma once
#include "IValueMap.h"

class BandValueMap final : public IValueMap
{
public:

	BandValueMap(float min, float max);

	__m256 operator[] (__m256 x) const override;

private:

	float min;
	float max;
};
