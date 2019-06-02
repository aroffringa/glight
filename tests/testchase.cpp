#include "../libtheatre/chase.h"
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

BOOST_AUTO_TEST_SUITE(chase)

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
	Chase& chase = management.AddChase();
	Sequence& sequence = chase.Sequence();
	root.Add(chase);
	sequence.Add(&pcA);
	sequence.Add(&pcB);
	BOOST_CHECK_EQUAL( management.Controllables().size(), 4); // 1 preset, 2 collections, 1 chase
	management.RemoveControllable(pcA);
	BOOST_CHECK_EQUAL( management.Controllables().size(), 2); // 1 preset, 1 collection, 0 chases
}

BOOST_AUTO_TEST_SUITE_END()
