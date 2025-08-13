#include "openfixturereader.h"

#include "../theatre/fixturemode.h"
#include "../theatre/fixturemodefunction.h"
#include "../theatre/fixturetype.h"
#include "../theatre/folder.h"
#include "../theatre/theatre.h"

namespace glight::system {

using theatre::FixtureMode;
using theatre::FixtureModeFunction;
using theatre::FixtureType;

namespace {
void ParseCapabilities(const json::Array& capabilities,
                       const std::string& channel_name,
                       std::map<std::string, FixtureModeFunction>& functions) {
  std::vector<std::pair<unsigned, unsigned>> empty_ranges;

  for (const json::Node& capability_child : capabilities) {
    const json::Object& capability = ToObj(capability_child);
    const json::Array& dmxRange = ToArr(capability["dmxRange"]);
    const unsigned start = ToNum(*dmxRange.items[0]).AsUInt();
    const unsigned end = ToNum(*dmxRange.items[1]).AsUInt() + 1;
    const std::string& type = ToStr(capability["type"]);
    FixtureModeFunction* function = nullptr;

    if (type == "NoFunction") {
      empty_ranges.emplace_back(start, end);

    } else if (type == "ColorPreset") {
      if (!function)
        function =
            &functions
                 .emplace(std::piecewise_construct,
                          std::make_tuple(channel_name),
                          std::make_tuple(theatre::FunctionType::ColorMacro, 0,
                                          OptionalNumber<size_t>(), 0))
                 .first->second;
      if (function->Type() == theatre::FunctionType::ColorMacro) {
        std::vector<glight::theatre::ColorRangeParameters::Range>& ranges =
            function->GetColorRangeParameters().GetRanges();
        if (!empty_ranges.empty()) {
          for (const std::pair<unsigned, unsigned>& r : empty_ranges) {
            ranges.emplace_back(r.first, r.second,
                                std::optional<theatre::Color>());
          }
          empty_ranges.clear();
        }
        const std::string& color_str =
            ToStr(*ToArr(capability["colors"]).items[0]);
        theatre::Color color =
            theatre::Color::FromHexString(color_str.c_str() + 1);
        ranges.emplace_back(start, end, color);
      }
    }
  }
}

std::map<std::string, FixtureModeFunction> ParseFunctions(
    const json::Object& fixture_object) {
  std::map<std::string, FixtureModeFunction> functions;
  const json::Object& channels = ToObj(fixture_object["availableChannels"]);
  for (const std::pair<const std::string, std::unique_ptr<json::Node>>& child :
       channels.children) {
    const std::string channel_name = child.first;
    const json::Object& availableChannel(ToObj(*child.second));
    const json::Object::const_iterator& capability_iter =
        availableChannel.find("capability");
    if (capability_iter != availableChannel.end()) {
      const json::Object& capability = ToObj(*capability_iter);
      const std::string& type = ToStr(capability["type"]);
      if (type == "Intensity") {
        functions.emplace(std::piecewise_construct,
                          std::make_tuple(channel_name),
                          std::make_tuple(theatre::FunctionType::Master, 0,
                                          OptionalNumber<size_t>(), 0));
      } else if (type == "ColorIntensity") {
        theatre::FunctionType t = theatre::FunctionType::Unknown;
        const std::string& color_str = ToStr(capability["color"]);
        if (color_str == "Red")
          t = theatre::FunctionType::Red;
        else if (color_str == "Green")
          t = theatre::FunctionType::Green;
        else if (color_str == "Blue")
          t = theatre::FunctionType::Blue;
        else if (color_str == "White")
          t = theatre::FunctionType::White;
        else if (color_str == "Amber")
          t = theatre::FunctionType::Amber;
        else if (color_str == "UV")
          t = theatre::FunctionType::UV;
        else if (color_str == "Lime")
          t = theatre::FunctionType::Lime;
        else if (color_str == "ColdWhite")
          t = theatre::FunctionType::ColdWhite;
        else if (color_str == "WarmWhite")
          t = theatre::FunctionType::WarmWhite;
        functions.emplace(std::piecewise_construct,
                          std::make_tuple(channel_name),
                          std::make_tuple(t, 0, OptionalNumber<size_t>(), 0));
      } else if (type == "EffectSpeed") {
        functions.emplace(std::piecewise_construct,
                          std::make_tuple(channel_name),
                          std::make_tuple(theatre::FunctionType::Strobe, 0,
                                          OptionalNumber<size_t>(), 0));
      }
    } else {
      const json::Array& capabilities = ToArr(availableChannel["capabilities"]);
      ParseCapabilities(capabilities, channel_name, functions);
    }
  }
  return functions;
}
}  // namespace

void ReadOpenFixture(theatre::Management& management, const json::Node& node) {
  const json::Object& fixture_object = ToObj(node);

  std::map<std::string, FixtureModeFunction> functions =
      ParseFunctions(fixture_object);
  const std::string fixture_name = ToStr(fixture_object["name"]);
  const json::Array& modes = ToArr(fixture_object["modes"]);
  FixtureType fixture_type(fixture_name);
  for (const json::Node& mode_node : modes) {
    const json::Object& mode = ToObj(mode_node);
    const std::string mode_name = ToStr(mode["name"]);
    const json::Array& mode_channels = ToArr(mode["channels"]);
    FixtureMode fixture_mode(fixture_type);
    fixture_mode.SetName(mode_name);
    std::vector<FixtureModeFunction> mode_functions;
    size_t dmx_channel = 0;
    for (const json::Node& channel : mode_channels) {
      const auto iter = functions.find(ToStr(channel));
      if (iter == functions.end())
        mode_functions.emplace_back(theatre::FunctionType::Unknown, dmx_channel,
                                    OptionalNumber<size_t>(), 0);
      else
        mode_functions.emplace_back(iter->second).SetDmxOffset(dmx_channel);
      if (mode_functions.back().FineChannelOffset())
        dmx_channel += 2;
      else
        ++dmx_channel;
    }
    fixture_mode.SetFunctions(mode_functions);
    system::ObservingPtr<FixtureMode> added_type =
        management.GetTheatre().AddFixtureType(fixture_type).GetObserver();
    management.RootFolder().Add(std::move(added_type));
  }
}

}  // namespace glight::system
