#pragma once
#include "IValueMap.h"

class BandValueMap : public IValueMap
{
public:

	BandValueMap(float min, float max);

	float operator[] (float x) const;

	__m256 operator[] (__m256 x) const;

private:

	float min;
	float max;
};
