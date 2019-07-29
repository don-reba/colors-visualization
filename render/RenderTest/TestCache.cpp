#include "Cache.h"

#include <boost/test/unit_test.hpp>

#include <functional>
#include <string>
#include <vector>

using namespace std;

namespace
{
	struct Mock
	{
		Mock(string & record) : record(record)
		{
			record += "c";
		}

		~Mock()
		{
			record += "d";
		}

		string & record;
	};

	struct TestCache
	{
		aligned_unique_ptr<Mock> MakeMock()
		{
			return make_aligned_unique<Mock>(record);
		}

		Cache<Mock> cache;
		string record;
	};
}

BOOST_FIXTURE_TEST_CASE(LoadOne, TestCache)
{
	cache.Load("test", [this]{ return MakeMock(); });
	BOOST_REQUIRE_EQUAL(record, "c");

	const Mock * data = cache.Load("test", [this]{ return MakeMock(); });
	BOOST_REQUIRE_EQUAL(record, "c");

	cache.Unload(data);
	BOOST_REQUIRE_EQUAL(record, "c");

	cache.Unload(data);
	BOOST_REQUIRE_EQUAL(record, "cd");
}

BOOST_FIXTURE_TEST_CASE(UnloadNonExistent, TestCache)
{
	BOOST_REQUIRE_THROW(cache.Unload(nullptr), logic_error);
}

BOOST_FIXTURE_TEST_CASE(IsEmpty, TestCache)
{
	BOOST_REQUIRE(cache.IsEmpty());

	cache.Load("test", [this]{ return MakeMock(); });

	BOOST_REQUIRE(!cache.IsEmpty());
}