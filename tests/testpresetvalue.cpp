#include "../theatre/presetcollection.h"
#include "../theatre/presetvalue.h"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(presetvalue)

BOOST_AUTO_TEST_CASE( SignalDelete )
{
	PresetCollection collection;
	std::unique_ptr<PresetValue> value(new PresetValue(collection, 37));
	bool isCalled = false;
	value->SignalDelete().connect( [&isCalled](){ isCalled = true; } );
	BOOST_CHECK_EQUAL(isCalled, false);
	value.reset();
	BOOST_CHECK_EQUAL(isCalled, true);
}

BOOST_AUTO_TEST_CASE( Copy )
{
	PresetCollection collection;
	PresetValue value(collection, 42);
	PresetValue copy(value);
	
	BOOST_CHECK_EQUAL(&copy.Controllable(), &collection);
	BOOST_CHECK_EQUAL(copy.InputIndex(), 42);
	
	bool isCalled = false;
	value.SignalDelete().connect( [&isCalled](){ isCalled = true; } );
	BOOST_CHECK_EQUAL(isCalled, false);

	std::unique_ptr<PresetValue> destructedCopy(new PresetValue(value));
	destructedCopy.reset();
	BOOST_CHECK_EQUAL(isCalled, false);
}

BOOST_AUTO_TEST_SUITE_END()

