#include "../theatre/fixturecontrol.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"
#include "../theatre/timing.h"

#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_SUITE(fixture_control)

BOOST_AUTO_TEST_CASE( Add )
{
	Management management;
	FixtureType& fixtureType = management.Theatre().AddFixtureType(FixtureType::RGBLight3Ch);
	Fixture& fixture = management.Theatre().AddFixture(fixtureType);
	FixtureControl& control = management.AddFixtureControl(fixture);
	BOOST_CHECK_EQUAL(&management.GetFixtureControl(fixture), &control);
	BOOST_CHECK_EQUAL(&control.Fixture(), &fixture);
	BOOST_CHECK_EQUAL(control.NInputs(), 3);
	BOOST_CHECK_EQUAL(control.NOutputs(), 0);
	BOOST_CHECK_NE(control.InputName(0), control.InputName(1));
	BOOST_CHECK_NE(control.InputName(1), control.InputName(2));
}

BOOST_AUTO_TEST_CASE( SetValue )
{
	Management management;
	FixtureType& fixtureType = management.Theatre().AddFixtureType(FixtureType::Light1Ch);
	Fixture& fixture = management.Theatre().AddFixture(fixtureType);
	fixture.SetChannel(100);
	FixtureControl& control = management.AddFixtureControl(fixture);
	control.InputValue(0) = ControlValue::Zero();
	control.MixInput(0, ControlValue::Max());
	std::vector<unsigned> values(512, 0);
	Timing timing(0.0, 0, 0, 0, 0);
	control.Mix(values.data(), 0, timing);
	for(size_t i=0; i!=512; ++i)
	{
		if(i == 100)
			BOOST_CHECK_EQUAL(values[100], ControlValue::MaxUInt());
		else
			BOOST_CHECK_EQUAL(values[i], 0);
	}
}

BOOST_AUTO_TEST_SUITE_END()
