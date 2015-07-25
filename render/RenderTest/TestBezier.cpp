#include "Bezier.h"

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

BOOST_AUTO_TEST_CASE(Bezier_Range)
{
	Bezier spline({0.93f, 0.55f}, {0.25f, 0.79f});
	float epsilon(0.001f);

	BOOST_CHECK_EQUAL(spline.Solve(0.0f, epsilon), 0.0f);
	BOOST_CHECK_EQUAL(spline.Solve(1.0f, epsilon), 1.0f);

	for (float x(0.0f); x <= 1.0f; x += 0.125f)
		BOOST_CHECK(InRange(spline.Solve(x, epsilon), 0.0f, 1.0f));
}

BOOST_AUTO_TEST_CASE(Bezier_Line)
{
	Bezier spline({0.1f, 0.1f}, {0.9f, 0.9f});
	float epsilon(0.001f);

	for (float x(0.0f); x <= 1.0f; x += 0.125f)
		BOOST_CHECK_CLOSE(x, spline.Solve(x, epsilon), 1.0f);
}