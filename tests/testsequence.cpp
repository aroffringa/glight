#include "../libtheatre/dummydevice.h"
#include "../libtheatre/management.h"
#include "../libtheatre/sequence.h"
#include "../libtheatre/theatre.h"
#include "../libtheatre/fixturecontrol.h"
#include "../libtheatre/presetvalue.h"
#include "../libtheatre/presetcollection.h"
#include "../libtheatre/folder.h"

#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_SUITE(sequence)

BOOST_AUTO_TEST_CASE( remove_indirect )
{
	Management management;
	management.AddDevice(std::unique_ptr<DmxDevice>(new DummyDevice()));
	Folder& root = management.RootFolder();
	FixtureType& ft = management.Theatre().AddFixtureType(FixtureType::Light1Ch);
	Fixture& f = management.Theatre().AddFixture(ft);
	FixtureControl& fc = management.AddFixtureControl(f);
	PresetValue& pv = management.AddPreset(fc, 0);
	pv.SetValue(ControlValue::Max());
	PresetCollection& pcA = management.AddPresetCollection();
	root.Add(pcA);
	PresetCollection& pcB = management.AddPresetCollection();
	root.Add(pcB);
	pcB.SetFromCurrentSituation(management);
	Sequence& sequence = management.AddSequence();
	root.Add(sequence);
	sequence.AddPreset(&pcA);
	sequence.AddPreset(&pcB);
	BOOST_CHECK_EQUAL( management.Sequences().size(), 1);
	management.RemoveControllable(pcA);
	BOOST_CHECK_EQUAL( management.Sequences().size(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

