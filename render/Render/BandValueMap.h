#pragma once
#include "IValueMap.h"

class BandValueMap : public IValueMap
{
public:

	BandValueMap(float min, float max);

	__m256 operator[] (__m256 x) const;

private:

	float min;
	float max;
};
