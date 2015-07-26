#include "BezierLookup.h"

#include <boost/test/unit_test.hpp>

using namespace boost;
using namespace Eigen;

namespace
{
	template<typename T>
	bool InRange(T x, T min, T max)
	{
		return x >= min && x <= max;
	}
}

BOOST_AUTO_TEST_CASE(BezierLookup_Range)
{
	for (size_t size(1); size != 0x100; size *= 2)
	{
		BezierLookup spline({0.93f, 0.55f}, {0.25f, 0.79f}, size);

		BOOST_CHECK_EQUAL(spline.size(), size);

		BOOST_CHECK_EQUAL(spline[0.0f], 0.0f);
		BOOST_CHECK_EQUAL(spline[1.0f], 1.0f);

		for (float x(0.0f); x <= 1.0f; x += 0.125f)
		{
			float y(spline[x]);
			BOOST_CHECK_MESSAGE(InRange(y, 0.0f, 1.0f), y << " not in [0,1] for " << x << " at " << size << " steps");
		}
	}
}
