#include "PiecewiseLinearValueMap.h"

#include <boost/test/unit_test.hpp>

using namespace std;

BOOST_AUTO_TEST_CASE(PiecewiseLinearValueMap_ThreePoints)
{
	constexpr float tolerance = 0.0001f;

	const PiecewiseLinearValueMap map({{0.0f, 0.1f}, {0.3f, 0.4f}, {1.0f, 1.8f}});

	for (const auto xy : PiecewiseLinearValueMap::Points
		{ {0.0f, 0.1f}, {0.1f, 0.2f}, {0.3f, 0.4f}
		, {0.4f, 0.6f}, {0.5f, 0.8f}, {0.6f, 1.0f}
		})
		BOOST_CHECK_CLOSE(map[get<0>(xy)], get<1>(xy), tolerance);
}