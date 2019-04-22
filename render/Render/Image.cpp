#include "Color.h"
#include "Image.h"

#include "FreeImagePlus.h"

#include <cstdio>
#include <stdexcept>

using namespace Eigen;
using namespace std;

namespace
{
	BYTE FloatToByteChannel(float x)
	{
		if (x <= 0.0f) return 0x00;
		if (x >= 1.0f) return 0xFF;
		// round to nearest
		return static_cast<BYTE>(x * 255.0f + 0.5f);
	}
}

void SaveProjectionBuffer
	( const char     * path
	, Resolution       res
	, const Vector4f * buffer
	)
{
	unsigned int bpp = 32;
	fipImage img(FIT_BITMAP, res.w, res.h, bpp);
	for (unsigned int y = 0; y != res.h; ++y)
	{
		BYTE * scanline(img.getScanLine(y));
		for (size_t x = 0; x != res.w; ++x)
		{
			const Vector4f & pxl = *buffer++;

			*scanline++ = pxl.x() == 0.0f ? 0x00 : 0xFF; // b
			*scanline++ = 0x00;                          // g
			*scanline++ = pxl.y() == 0.0f ? 0x00 : 0xFF; // r
			*scanline++ = 0xFF;
		}
	}
	img.save(path);
}

void SaveBuffer
	( const char     * path
	, Resolution       res
	, const Vector4f * buffer
	, Vector3f         bgColor
	)
{
	unsigned int bpp = 32;
	fipImage img(FIT_BITMAP, res.w, res.h, bpp);
	for (unsigned int y = 0; y != res.h; ++y)
	{
		BYTE * scanline(img.getScanLine(y));
		for (size_t x = 0; x != res.w; ++x)
		{
			const Vector4f & pxl = *buffer++;

			const Vector3f fgColor(pxl.x(), pxl.y(), pxl.z());
			const Vector3f lab(bgColor + pxl.w() * (fgColor - bgColor));
			const Vector3f rgb(LabToRgb(lab));

			*scanline++ = FloatToByteChannel(rgb(2)); // b
			*scanline++ = FloatToByteChannel(rgb(1)); // g
			*scanline++ = FloatToByteChannel(rgb(0)); // r
			*scanline++ = 0xFF;
		}
	}
	img.save(path);
}