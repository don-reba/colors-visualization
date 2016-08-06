#include "pow.h"

#include <cmath>

#include <boost/test/unit_test.hpp>

using namespace std;

namespace
{
	float log2f4to1(float x)
	{
		__declspec(align(16)) float v[4];
		_mm_store_ps(v, log2f4(_mm_set1_ps(x)));
		return v[0];
	}

	float log2f8to1(float x)
	{
		__declspec(align(32)) float v[8];
		_mm256_store_ps(v, log2f8(_mm256_set1_ps(x)));
		return v[0];
	}
}

BOOST_AUTO_TEST_CASE(Log2)
{
	const float tolerance = 0.002f;

	const float args[] = { 0.1f, 0.5f, 1.0f, 2.0f, 5.0f, 100.0f };
	for (float arg : args)
	{
		BOOST_CHECK_CLOSE(log2f4to1(arg), log2(arg), tolerance);
		BOOST_CHECK_CLOSE(log2f8to1(arg), log2(arg), tolerance);
	}
}

BOOST_AUTO_TEST_CASE(Log2f4_Fullness)
{
	const float x = 0.1f;

	__declspec(align(16)) float v[4];
	_mm_store_ps(v, log2f4(_mm_set1_ps(x)));

	for (size_t i = 1; i != 4; ++i)
		BOOST_CHECK_EQUAL(v[0], v[i]);
}

BOOST_AUTO_TEST_CASE(Log2f8_Fullness)
{
	const float x = 0.1f;

	__declspec(align(32)) float v[8];
	_mm256_store_ps(v, log2f8(_mm256_set1_ps(x)));

	for (size_t i = 1; i != 8; ++i)
		BOOST_CHECK_EQUAL(v[0], v[i]);
}