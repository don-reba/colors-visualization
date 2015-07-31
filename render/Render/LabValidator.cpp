#include "LabValidator.h"

using namespace Eigen;

LabValidator::LabValidator(size_t lookupTableSize)
	: t(lookupTableSize)
{
	const float min(-1.80f), max(4.1f);

	for (size_t i(0); i != lookupTableSize; ++i)
	{
		const float d = i / (lookupTableSize - 1.0f);
		const float x = (1.0f - d) * min + d * max;

		const float value = (x > 0.0031308f) ? 1.055f * pow(x, 1.0f / 2.4f) - 0.055f : x * 12.92f;
		t[i] = value < 0 || value > 1;
	}

	this->offset = -min; this->factor = (lookupTableSize - 1.0f) / (max - min);
}

int LabValidator::LookUp(float x) const
{
	const float position = factor * (x + offset);
	int index;
	// sensitive to rounding mode
	__asm
	{
		fld    position
		fisttp index
	}
	return t[index];
}

bool LabValidator::Check(Vector3f lab)
{
	// adapted from http://www.easyrgb.com

	// Lab to XYZ
	// Observer. = 2°; Illuminant = D65
	float l_ = lab(0);
	float a_ = lab(1);
	float b_ = lab(2);

	float y =  (l_ + 16.0f) / 116.0f;
	float x = a_ / 500.0f + y;
	float z = y - b_ / 200.0f;

	x  = (x * x * x > 0.008856f) ? x * x * x  : (x - 16.0f / 116.0f) / 7.787f;
	y  = (y * y * y > 0.008856f) ? y * y * y  : (y - 16.0f / 116.0f) / 7.787f;
	z  = (z * z * z > 0.008856f) ? z * z * z  : (z - 16.0f / 116.0f) / 7.787f;

	// multiply by reference point
	x *= 0.95047f;
	y *= 1.00000f;
	z *= 1.08883f;

	// XYZ to sRGB
	float r = x *  3.2406f + y * -1.5372f + z * -0.4986f;
	float g = x * -0.9689f + y *  1.8758f + z *  0.0415f;
	float b = x *  0.0557f + y * -0.2040f + z *  1.0570f;

	if (r < -1.75227319813683f   || r > 3.68007519831531f) return false;
	if (g < -0.0806535781744183f || g > 1.51646331806568f) return false;
	if (b < -0.0706201022919949f || b > 4.04958281711443f) return false;

	if (LookUp(r)) return false;
	if (LookUp(g)) return false;
	if (LookUp(b)) return false;

	return true;
}
