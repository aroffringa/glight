#include "../theatre/chase.h"
#include "../theatre/dummydevice.h"
#include "../theatre/fixturecontrol.h"
#include "../theatre/folder.h"
#include "../theatre/management.h"
#include "../theatre/presetcollection.h"
#include "../theatre/presetvalue.h"
#include "../theatre/sequence.h"
#include "../theatre/theatre.h"

#include <boost/test/unit_test.hpp>

#include <memory>

BOOST_AUTO_TEST_SUITE(chase)

BOOST_AUTO_TEST_CASE(remove_indirect) {
  Management management;
  management.AddDevice(std::unique_ptr<DmxDevice>(new DummyDevice()));
  Folder &root = management.RootFolder();
  FixtureType &ft =
      management.GetTheatre().AddFixtureType(StockFixture::Light1Ch);
  Fixture &f = management.GetTheatre().AddFixture(ft);
  FixtureControl &fc = management.AddFixtureControl(f);
  SourceValue &sv = management.AddSourceValue(fc, 0);
  sv.Preset().SetValue(ControlValue::Max());
  PresetCollection &pcA = management.AddPresetCollection();
  pcA.SetName("pcA");
  root.Add(pcA);
  PresetCollection &pcB = management.AddPresetCollection();
  pcB.SetName("pcB");
  root.Add(pcB);
  pcB.SetFromCurrentSituation(management);
  Chase &chase = management.AddChase();
  chase.SetName("chase");
  Sequence &sequence = chase.Sequence();
  root.Add(chase);
  sequence.Add(pcA, 0);
  sequence.Add(pcB, 0);
  BOOST_CHECK_EQUAL(management.Controllables().size(),
                    4);  // 1 preset, 2 collections, 1 chase
  management.RemoveControllable(pcA);
  BOOST_CHECK_EQUAL(management.Controllables().size(),
                    2);  // 1 preset, 1 collection, 0 chases
}

BOOST_AUTO_TEST_SUITE_END()
