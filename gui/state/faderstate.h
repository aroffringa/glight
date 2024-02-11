#ifndef GLIGHT_FADER_STATE_H_
#define GLIGHT_FADER_STATE_H_

#include <string>
#include <vector>

#include <sigc++/connection.h>
#include <sigc++/signal.h>

namespace glight::theatre {
class Management;
class SourceValue;
}  // namespace glight::theatre

namespace glight::gui {

enum class FaderControlType { Fader, ToggleButton, ColorButton };

constexpr inline bool IsFullColumnType(FaderControlType fader_type) {
  return fader_type == FaderControlType::Fader;
}

constexpr inline FaderControlType GetFaderControlType(
    const std::string &fader_type_str) {
  if (fader_type_str == "fader")
    return FaderControlType::Fader;
  else if (fader_type_str == "toggle-button")
    return FaderControlType::ToggleButton;
  else if (fader_type_str == "color-button")
    return FaderControlType::ColorButton;
  else
    throw std::runtime_error("Invalid fader control type");
}

inline std::string ToString(FaderControlType fader_type) {
  switch (fader_type) {
    case FaderControlType::Fader:
      return "fader";
    case FaderControlType::ToggleButton:
      return "toggle-button";
    case FaderControlType::ColorButton:
      return "color-button";
    default:
      return "";
  }
}

class FaderState {
 public:
  FaderState() = default;
  explicit FaderState(std::vector<theatre::SourceValue *> source_values);
  FaderState(const FaderState &source) = delete;
  ~FaderState();

  FaderState &operator=(const FaderState &rhs) = delete;

  void SetSourceValues(std::vector<theatre::SourceValue *> source_values);
  FaderControlType GetFaderType() const { return fader_type_; }
  void SetFaderType(FaderControlType fader_type) {
    if (fader_type != fader_type_) {
      fader_type_ = fader_type;
      signal_change_();
    }
  }
  void SetColumn(bool new_column) {
    if (new_column != new_column_) {
      new_column_ = new_column;
      signal_change_();
    }
  }

  const std::vector<theatre::SourceValue *> &GetSourceValues() const {
    return source_values_;
  }
  bool NewToggleButtonColumn() const { return new_column_; }

  bool DisplayName() const { return display_name_; }
  void SetDisplayName(bool display_name) {
    if (display_name_ != display_name) {
      display_name_ = display_name;
      signal_change_();
    }
  }

  bool DisplayFlashButton() const { return display_flash_button_; }
  void SetDisplayFlashButton(bool display_flash_button) {
    if (display_flash_button_ != display_flash_button) {
      display_flash_button_ = display_flash_button;
      signal_change_();
    }
  }

  bool DisplayCheckButton() const { return display_check_button_; }
  void SetDisplayCheckButton(bool display_check_button) {
    if (display_check_button != display_check_button_) {
      display_check_button_ = display_check_button;
      signal_change_();
    }
  }

  bool OverlayFadeButtons() const { return overlay_fade_buttons_; }
  void SetOverlayFadeButtons(bool overlay_fade_buttons) {
    if (overlay_fade_buttons != overlay_fade_buttons_) {
      overlay_fade_buttons_ = overlay_fade_buttons;
      signal_change_();
    }
  }

  sigc::signal<void()> &SignalChange() { return signal_change_; }

 private:
  void Connect();
  void Disconnect();
  void onPresetValueDeleted();

  std::vector<theatre::SourceValue *> source_values_;
  FaderControlType fader_type_ = FaderControlType::Fader;
  bool new_column_ = false;
  bool display_name_ = true;
  bool display_flash_button_ = true;
  bool display_check_button_ = true;
  bool overlay_fade_buttons_ = true;
  sigc::signal<void()> signal_change_;
  std::vector<sigc::connection> source_value_deleted_connections_;
};

}  // namespace glight::gui

#endif
