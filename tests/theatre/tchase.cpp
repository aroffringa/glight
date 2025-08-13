#include "theatre/chase.h"
#include "theatre/fixturecontrol.h"
#include "theatre/fixturetype.h"
#include "theatre/folder.h"
#include "theatre/management.h"
#include "theatre/presetcollection.h"
#include "theatre/presetvalue.h"
#include "theatre/sequence.h"
#include "theatre/theatre.h"

#include "system/settings.h"

#include <boost/test/unit_test.hpp>

#include <memory>

using namespace glight::theatre;
using glight::system::ObservingPtr;

BOOST_AUTO_TEST_SUITE(chase)

BOOST_AUTO_TEST_CASE(remove_indirect) {
  const glight::system::Settings settings;
  Management management(settings);
  Folder &root = management.RootFolder();
  ObservingPtr<FixtureType> ft =
      management.GetTheatre().AddFixtureTypePtr(StockFixture::Light1Ch);
  Fixture &f = *management.GetTheatre().AddFixture(ft->Modes().front());
  ObservingPtr<FixtureControl> fc = management.AddFixtureControlPtr(f);
  SourceValue &sv = management.AddSourceValue(*fc, 0);
  sv.A().SetValue(ControlValue::Max());
  ObservingPtr<PresetCollection> pcA = management.AddPresetCollectionPtr();
  pcA->SetName("pcA");
  root.Add(pcA);
  ObservingPtr<PresetCollection> pcB = management.AddPresetCollectionPtr();
  pcB->SetName("pcB");
  root.Add(pcB);
  pcB->SetFromCurrentSituation(management);
  ObservingPtr<Chase> chase = management.AddChasePtr();
  chase->SetName("chase");
  Sequence &sequence = chase->GetSequence();
  root.Add(chase);
  sequence.Add(*pcA, 0);
  sequence.Add(*pcB, 0);
  BOOST_CHECK_EQUAL(management.Controllables().size(),
                    4);  // 1 preset, 2 collections, 1 chase
  management.RemoveControllable(*pcA);
  BOOST_CHECK_EQUAL(management.Controllables().size(),
                    2);  // 1 preset, 1 collection, 0 chases
}

BOOST_AUTO_TEST_SUITE_END()
