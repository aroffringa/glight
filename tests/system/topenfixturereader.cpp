#include "system/openfixturereader.h"
#include "system/settings.h"

#include "theatre/fixturetypefunction.h"
#include "theatre/management.h"
#include "theatre/theatre.h"

#include <boost/test/unit_test.hpp>

#include <sstream>

namespace glight {

using system::ReadOpenFixture;
using theatre::FunctionType;
using theatre::Management;

namespace {
const char* kFlatParQA12 = R"(
{
  "$schema": "https://raw.githubusercontent.com/OpenLightingProject/open-fixture-library/master/schemas/fixture.json",
  "name": "Flat Par QA12",
  "shortName": "QA12",
  "categories": ["Color Changer"],
  "meta": {
    "authors": ["TAKU", "Flo Edelmann", "Ken Harris"],
    "createDate": "2022-03-20",
    "lastModifyDate": "2022-03-20"
  },
  "links": {
    "manual": [
      "https://d295jznhem2tn9.cloudfront.net/ItemRelatedFiles/7028/flat_par_qa12.pdf"
    ],
    "productPage": [
      "https://www.adj.com/flat-par-qa12"
    ]
  },
  "physical": {
    "dimensions": [280, 115, 328],
    "weight": 3.2,
    "power": 80,
    "DMXconnector": "3-pin",
    "bulb": {
      "type": "12× 5W RGBA LED"
    },
    "lens": {
      "degreesMinMax": [40, 40]
    }
  },
  "availableChannels": {
    "Color Macros": {
      "capabilities": [
        {
          "dmxRange": [0, 15],
          "type": "NoFunction"
        },
        {
          "dmxRange": [16, 31],
          "type": "ColorPreset",
          "colors": ["#ff0000"],
          "comment": "Red"
        },
        {
          "dmxRange": [32, 47],
          "type": "ColorPreset",
          "colors": ["#00ff00"],
          "comment": "Green"
        },
        {
          "dmxRange": [48, 63],
          "type": "ColorPreset",
          "colors": ["#0000ff"],
          "comment": "Blue"
        },
        {
          "dmxRange": [64, 79],
          "type": "ColorPreset",
          "colors": ["#ffbf00"],
          "comment": "Amber"
        },
        {
          "dmxRange": [80, 95],
          "type": "ColorPreset",
          "colors": ["#ffff00"],
          "comment": "Yellow"
        },
        {
          "dmxRange": [96, 111],
          "type": "ColorPreset",
          "colors": ["#ff00ff"],
          "comment": "Pink"
        },
        {
          "dmxRange": [112, 127],
          "type": "ColorPreset",
          "colors": ["#ff6000"],
          "comment": "Orange"
        },
        {
          "dmxRange": [128, 143],
          "type": "ColorPreset",
          "colors": ["#00ffff"],
          "comment": "Cyan"
        },
        {
          "dmxRange": [144, 159],
          "type": "ColorPreset",
          "colors": ["#92ff00"],
          "comment": "Lime"
        },
        {
          "dmxRange": [160, 175],
          "type": "ColorPreset",
          "colors": ["#ffbfff"],
          "comment": "Lavender"
        },
        {
          "dmxRange": [176, 191],
          "type": "ColorPreset",
          "colors": ["#ffffff"],
          "comment": "White"
        },
        {
          "dmxRange": [192, 207],
          "type": "ColorPreset",
          "colors": ["#ffdf00"],
          "comment": "Warm Yellow"
        },
        {
          "dmxRange": [208, 223],
          "type": "ColorPreset",
          "colors": ["#ff6080"],
          "comment": "Light Red"
        },
        {
          "dmxRange": [224, 239],
          "type": "ColorPreset",
          "colors": ["#92ff92"],
          "comment": "Light Green"
        },
        {
          "dmxRange": [240, 255],
          "type": "ColorPreset",
          "colors": ["#ffdf80"],
          "comment": "Light Amber"
        }
      ]
    },
    "Dimmer": {
      "capability": {
        "type": "Intensity"
      }
    },
    "Strobe": {
      "capabilities": [
        {
          "dmxRange": [0, 15],
          "type": "NoFunction"
        },
        {
          "dmxRange": [16, 255],
          "type": "ShutterStrobe",
          "shutterEffect": "Strobe",
          "speedStart": "slow",
          "speedEnd": "fast"
        }
      ]
    },
    "Red": {
      "capability": {
        "type": "ColorIntensity",
        "color": "Red"
      }
    },
    "Green": {
      "capability": {
        "type": "ColorIntensity",
        "color": "Green"
      }
    },
    "Blue": {
      "capability": {
        "type": "ColorIntensity",
        "color": "Blue"
      }
    },
    "Amber": {
      "capability": {
        "type": "ColorIntensity",
        "color": "Amber"
      }
    },
    "Program Speed": {
      "capability": {
        "type": "EffectSpeed",
        "speedStart": "slow",
        "speedEnd": "fast"
      }
    },
    "Sound Sensitivity": {
      "capabilities": [
        {
          "dmxRange": [0, 31],
          "type": "SoundSensitivity",
          "soundSensitivity": "off"
        },
        {
          "dmxRange": [32, 255],
          "type": "SoundSensitivity",
          "soundSensitivityStart": "low",
          "soundSensitivityEnd": "high"
        }
      ]
    },
    "Mode Select": {
      "defaultValue": 0,
      "capabilities": [
        {
          "dmxRange": [0, 51],
          "type": "Maintenance",
          "comment": "Dimmer mode",
          "switchChannels": {
            "Strobe / Speed / Sensitivity": "Strobe",
            "Color Macros / Programs": "Unused"
          }
        },
        {
          "dmxRange": [52, 102],
          "type": "Maintenance",
          "comment": "Color macro mode",
          "switchChannels": {
            "Strobe / Speed / Sensitivity": "Strobe",
            "Color Macros / Programs": "Color Macros"
          }
        },
        {
          "dmxRange": [103, 153],
          "type": "Maintenance",
          "comment": "Color change mode",
          "switchChannels": {
            "Strobe / Speed / Sensitivity": "Program Speed",
            "Color Macros / Programs": "Color Change Programs"
          }
        },
        {
          "dmxRange": [154, 204],
          "type": "Maintenance",
          "comment": "Color fade mode",
          "switchChannels": {
            "Strobe / Speed / Sensitivity": "Program Speed",
            "Color Macros / Programs": "Color Fade Programs"
          }
        },
        {
          "dmxRange": [205, 255],
          "type": "Maintenance",
          "comment": "Sound active mode",
          "switchChannels": {
            "Strobe / Speed / Sensitivity": "Sound Sensitivity",
            "Color Macros / Programs": "Sound Active Programs"
          }
        }
      ]
    },
    "Color Change Programs": {
      "capabilities": [
        {
          "dmxRange": [0, 15],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "1"
        },
        {
          "dmxRange": [16, 31],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "2"
        },
        {
          "dmxRange": [32, 47],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "3"
        },
        {
          "dmxRange": [48, 63],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "4"
        },
        {
          "dmxRange": [64, 79],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "5"
        },
        {
          "dmxRange": [80, 95],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "6"
        },
        {
          "dmxRange": [96, 111],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "7"
        },
        {
          "dmxRange": [112, 127],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "8"
        },
        {
          "dmxRange": [128, 143],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "9"
        },
        {
          "dmxRange": [144, 159],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "10"
        },
        {
          "dmxRange": [160, 175],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "11"
        },
        {
          "dmxRange": [176, 191],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "12"
        },
        {
          "dmxRange": [192, 207],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "13"
        },
        {
          "dmxRange": [208, 223],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "14"
        },
        {
          "dmxRange": [224, 239],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "15"
        },
        {
          "dmxRange": [240, 255],
          "type": "Effect",
          "effectPreset": "ColorJump",
          "comment": "16"
        }
      ]
    },
    "Color Fade Programs": {
      "capabilities": [
        {
          "dmxRange": [0, 15],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "1"
        },
        {
          "dmxRange": [16, 31],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "2"
        },
        {
          "dmxRange": [32, 47],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "3"
        },
        {
          "dmxRange": [48, 63],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "4"
        },
        {
          "dmxRange": [64, 79],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "5"
        },
        {
          "dmxRange": [80, 95],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "6"
        },
        {
          "dmxRange": [96, 111],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "7"
        },
        {
          "dmxRange": [112, 127],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "8"
        },
        {
          "dmxRange": [128, 143],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "9"
        },
        {
          "dmxRange": [144, 159],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "10"
        },
        {
          "dmxRange": [160, 175],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "11"
        },
        {
          "dmxRange": [176, 191],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "12"
        },
        {
          "dmxRange": [192, 207],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "13"
        },
        {
          "dmxRange": [208, 223],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "14"
        },
        {
          "dmxRange": [224, 239],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "15"
        },
        {
          "dmxRange": [240, 255],
          "type": "Effect",
          "effectPreset": "ColorFade",
          "comment": "16"
        }
      ]
    },
    "Sound Active Programs": {
      "capabilities": [
        {
          "dmxRange": [0, 15],
          "type": "Effect",
          "effectName": "Sound active program 1",
          "soundControlled": true
        },
        {
          "dmxRange": [16, 31],
          "type": "Effect",
          "effectName": "Sound active program 2",
          "soundControlled": true
        },
        {
          "dmxRange": [32, 47],
          "type": "Effect",
          "effectName": "Sound active program 3",
          "soundControlled": true
        },
        {
          "dmxRange": [48, 63],
          "type": "Effect",
          "effectName": "Sound active program 4",
          "soundControlled": true
        },
        {
          "dmxRange": [64, 79],
          "type": "Effect",
          "effectName": "Sound active program 5",
          "soundControlled": true
        },
        {
          "dmxRange": [80, 95],
          "type": "Effect",
          "effectName": "Sound active program 6",
          "soundControlled": true
        },
        {
          "dmxRange": [96, 111],
          "type": "Effect",
          "effectName": "Sound active program 7",
          "soundControlled": true
        },
        {
          "dmxRange": [112, 127],
          "type": "Effect",
          "effectName": "Sound active program 8",
          "soundControlled": true
        },
        {
          "dmxRange": [128, 143],
          "type": "Effect",
          "effectName": "Sound active program 9",
          "soundControlled": true
        },
        {
          "dmxRange": [144, 159],
          "type": "Effect",
          "effectName": "Sound active program 10",
          "soundControlled": true
        },
        {
          "dmxRange": [160, 175],
          "type": "Effect",
          "effectName": "Sound active program 11",
          "soundControlled": true
        },
        {
          "dmxRange": [176, 191],
          "type": "Effect",
          "effectName": "Sound active program 12",
          "soundControlled": true
        },
        {
          "dmxRange": [192, 207],
          "type": "Effect",
          "effectName": "Sound active program 13",
          "soundControlled": true
        },
        {
          "dmxRange": [208, 223],
          "type": "Effect",
          "effectName": "Sound active program 14",
          "soundControlled": true
        },
        {
          "dmxRange": [224, 239],
          "type": "Effect",
          "effectName": "Sound active program 15",
          "soundControlled": true
        },
        {
          "dmxRange": [240, 255],
          "type": "Effect",
          "effectName": "Sound active program 16",
          "soundControlled": true
        }
      ]
    },
    "Unused": {
      "capability": {
        "type": "NoFunction"
      }
    }
  },
  "modes": [
    {
      "name": "1-channel",
      "shortName": "1ch",
      "channels": [
        "Color Macros"
      ]
    },
    {
      "name": "2-channel",
      "shortName": "2ch",
      "channels": [
        "Color Macros",
        "Dimmer"
      ]
    },
    {
      "name": "3-channel",
      "shortName": "3ch",
      "channels": [
        "Color Macros",
        "Dimmer",
        "Strobe"
      ]
    },
    {
      "name": "4-channel",
      "shortName": "4ch",
      "channels": [
        "Red",
        "Green",
        "Blue",
        "Amber"
      ]
    },
    {
      "name": "5-channel",
      "shortName": "5ch",
      "channels": [
        "Red",
        "Green",
        "Blue",
        "Amber",
        "Dimmer"
      ]
    },
    {
      "name": "6-channel",
      "shortName": "6ch",
      "channels": [
        "Red",
        "Green",
        "Blue",
        "Amber",
        "Dimmer",
        "Color Macros"
      ]
    },
    {
      "name": "7-channel",
      "shortName": "7ch",
      "channels": [
        "Red",
        "Green",
        "Blue",
        "Amber",
        "Dimmer",
        "Strobe",
        "Color Macros"
      ]
    },
    {
      "name": "8-channel",
      "shortName": "8ch",
      "channels": [
        "Red",
        "Green",
        "Blue",
        "Amber",
        "Dimmer",
        "Strobe / Speed / Sensitivity",
        "Mode Select",
        "Color Macros / Programs"
      ]
    }
  ]
}
)";
}

BOOST_AUTO_TEST_SUITE(open_fixture_reader)

BOOST_AUTO_TEST_CASE(read) {
  const glight::system::Settings settings;
  Management management(settings);
  std::istringstream data_stream(kFlatParQA12);
  std::unique_ptr<json::Node> root = json::Parse(data_stream);
  theatre::Theatre& theatre = management.GetTheatre();
  ReadOpenFixture(management, *root);
  system::ObservingPtr<theatre::FixtureType> f1 =
      theatre.GetFixtureType("Flat Par QA12 (1-channel)")
          .GetObserver<theatre::FixtureType>();
  BOOST_REQUIRE_EQUAL(f1->Functions().size(), 1);
  BOOST_REQUIRE(f1->Functions()[0].Type() == theatre::FunctionType::ColorMacro);
  const theatre::ColorRangeParameters& parameters =
      f1->Functions()[0].GetColorRangeParameters();
  BOOST_CHECK_EQUAL(parameters.GetRanges().size(), 16);
  BOOST_CHECK(!parameters.GetRanges()[0].color);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[0].input_min, 0);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[0].input_max, 16);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[1].input_min, 16);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[1].input_max, 32);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[1].color->Red(), 255);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[1].color->Green(), 0);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[1].color->Blue(), 0);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[2].input_min, 32);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[2].input_max, 48);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[2].color->Red(), 0);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[2].color->Green(), 255);
  BOOST_CHECK_EQUAL(parameters.GetRanges()[2].color->Blue(), 0);
}

BOOST_AUTO_TEST_SUITE_END()

}  // namespace glight
