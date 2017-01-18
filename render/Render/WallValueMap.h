#pragma once
#include "IValueMap.h"

class WallValueMap : public IValueMap
{
public:

	WallValueMap(float min, float max);

	float operator[] (float x) const;

	__m256 operator[] (__m256 x) const;

private:

	float min;
	float max;
};
