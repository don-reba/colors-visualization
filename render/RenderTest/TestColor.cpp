#include "Color.h"

#include <vector>
#include <utility>

#include <boost/test/unit_test.hpp>

using namespace boost;
using namespace Eigen;
using namespace std;

namespace
{
	bool EqualToWithin
		( const Vector3f & a 
		, const Vector3f & b
		, float margin
		)
	{
		return (a - b).norm() < margin;
	}
}

BOOST_AUTO_TEST_CASE(Color_Test)
{
	const vector<pair<Vector3f, Vector3f>> pairs =
	{ { { 87.105f,  -0.199f, 64.623f }, { 0.99216f, 0.84314f, 0.35294f } }
	, { { 75.474f,  22.602f, 58.234f }, { 0.98431f, 0.65882f, 0.29804f } }
	, { { 62.352f,  45.269f, 26.129f }, { 0.91765f, 0.45098f, 0.41961f } }
	, { { 51.756f,  23.715f, 19.664f }, { 0.67059f, 0.41961f, 0.35686f } }
	, { { 65.478f, -10.766f, 33.501f }, { 0.63137f, 0.63922f, 0.38431f } } 
	};
	for (const auto & pair : pairs)
	{
		const Vector3f & lab(pair.first);
		const Vector3f & rgb(pair.second);
		BOOST_CHECK_MESSAGE
			( EqualToWithin(::LabToRgb(lab), rgb, 0.0001f)
			, ::LabToRgb(lab).transpose() << " not equal to " << rgb.transpose()
			);
		BOOST_CHECK_MESSAGE
			( EqualToWithin(::RgbToLab(rgb), lab, 0.01f)
			, lab.transpose() << " not equal to " << ::RgbToLab(rgb).transpose()
			);
	}
}

BOOST_AUTO_TEST_CASE(Color_Valid)
{
	const float offset(0.001f);
	const float a(0.0f + offset), b(1.0f - offset);
	const float c(0.0f - offset), d(1.0f + offset);

	for (float x(a); x <= b; x += 0.1f)
	for (float y(a); y <= b; y += 0.1f)
	{
		Vector3f valid[6] =
			{ {a, x, y}, {b, x, y}
			, {x, a, y}, {x, b, y}
			, {x, y, a}, {x, y, b}
			};
		for (Vector3f rgb : valid)
		{
			BOOST_CHECK_MESSAGE
				( ::IsValidLab(::RgbToLab(rgb))
				, rgb.transpose() << " tests invalid"
				);
		}

		Vector3f invalid[6] =
			{ {c, x, y}, {d, x, y}
			, {x, c, y}, {x, d, y}
			, {x, y, c}, {x, y, d}
			};
		for (Vector3f rgb : invalid)
		{
			BOOST_CHECK_MESSAGE
				( !::IsValidLab(::RgbToLab(rgb))
				, rgb.transpose() << " tests valid"
				);
		}
	}
}
