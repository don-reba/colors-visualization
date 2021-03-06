#include "Color.h"

#include <fstream>
#include <vector>

using namespace Eigen;

Vector3f LabToRgb(Vector3f lab)
{
	// adapted from http://www.easyrgb.com

	// Lab to XYZ
	// Observer. = 2�; Illuminant = D65
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

	r = (r > 0.0031308f) ? 1.055f * pow(r, 1.0f / 2.4f) - 0.055f : r * 12.92f;
	g = (g > 0.0031308f) ? 1.055f * pow(g, 1.0f / 2.4f) - 0.055f : g * 12.92f;
	b = (b > 0.0031308f) ? 1.055f * pow(b, 1.0f / 2.4f) - 0.055f : b * 12.92f;

	return Vector3f(r, g, b);
}

Vector3f RgbToLab(Vector3f rgb)
{
	// adapted from http://www.easyrgb.com

	float r = rgb(0);
	float g = rgb(1);
	float b = rgb(2);

	// compand sRGB
	r = (r > 0.04045) ? pow((r + 0.055f) / 1.055f, 2.4f) : r / 12.92f;
	g = (g > 0.04045) ? pow((g + 0.055f) / 1.055f, 2.4f) : g / 12.92f;
	b = (b > 0.04045) ? pow((b + 0.055f) / 1.055f, 2.4f) : b / 12.92f;

	// sRGB to XYZ
	// Observer. = 2�; Illuminant = D65
	float x = r * 0.4124f + g * 0.3576f + b * 0.1805f;
	float y = r * 0.2126f + g * 0.7152f + b * 0.0722f;
	float z = r * 0.0193f + g * 0.1192f + b * 0.9505f;

	// XYZ to Lab
	// divide by reference point
	x /= 0.95047f;
	y /= 1.00000f;
	z /= 1.08883f;

	x = (x > 0.008856) ? pow(x, 1.0f / 3.0f) : 7.787f * x + 16.0f / 116.0f;
	y = (y > 0.008856) ? pow(y, 1.0f / 3.0f) : 7.787f * y + 16.0f / 116.0f;
	z = (z > 0.008856) ? pow(z, 1.0f / 3.0f) : 7.787f * z + 16.0f / 116.0f;

	float l_ = (116.0f * y) - 16.0f;
	float a_ = 500.0f * (x - y);
	float b_ = 200.0f * (y - z);

	return Vector3f(l_, a_, b_);
}

bool IsValidLab(Vector3f lab)
{
	// adapted from http://www.easyrgb.com

	// Lab to XYZ
	// Observer. = 2�; Illuminant = D65
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
	const float r = x *  3.2406f + y * -1.5372f + z * -0.4986f;
	const float g = x * -0.9689f + y *  1.8758f + z *  0.0415f;
	const float b = x *  0.0557f + y * -0.2040f + z *  1.0570f;

	// skip a monotonous-increasing transformation from [0,1] to [0,1]
	if (r < 0.0f || r > 1.0f) return false;
	if (g < 0.0f || g > 1.0f) return false;
	if (b < 0.0f || b > 1.0f) return false;

	return true;
}