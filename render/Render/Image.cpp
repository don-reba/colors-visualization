#include "Color.h"
#include "Image.h"

#include "FreeImagePlus.h"

#include <cstdio>
#include <stdexcept>

using namespace Eigen;
using namespace std;

BYTE FloatToByteChannel(float x)
{
	if (x <= 0.0f) return 0x00;
	if (x >= 1.0f) return 0xFF;
	return static_cast<BYTE>(x * 255.0f);
}

void SaveBuffer(const char * path, size_t width, size_t height, const Vector4f * buffer)
{
	size_t bpp(32);
	fipImage img(FIT_BITMAP, width, height, bpp);
	for (size_t y(0); y != height; ++y)
	{
		BYTE * scanline(img.getScanLine(y));
		for (size_t x(0); x !=  width; ++x)
		{
			const Vector4f & pxl(*buffer++);
			Vector3f rgb(LabToRgb(Vector3f(pxl.x(), pxl.y(), pxl.z())));
			*scanline++ = FloatToByteChannel(rgb(2));
			*scanline++ = FloatToByteChannel(rgb(1));
			*scanline++ = FloatToByteChannel(rgb(0));
			*scanline++ = FloatToByteChannel(pxl.w());
		}
	}
	img.save(path);
}