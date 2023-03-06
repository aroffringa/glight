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

class FaderState {
 public:
  explicit FaderState(theatre::SourceValue *source_value);
  FaderState() : is_toggle_button_(false), new_toggle_button_column_(false) {}
  FaderState(const FaderState &source) = delete;
  ~FaderState() { source_value_deleted_connection_.disconnect(); }

  FaderState &operator=(const FaderState &rhs) = delete;

  void SetSourceValue(theatre::SourceValue *value);
  void SetNoSourceValue() {
    SetSourceValue(nullptr);
    signal_change_();
  }
  void SetIsToggleButton(bool is_toggle) {
    if (is_toggle != is_toggle_button_) {
      is_toggle_button_ = is_toggle;
      signal_change_();
    }
  }
  void SetNewToggleButtonColumn(bool new_column) {
    if (new_column != new_toggle_button_column_) {
      new_toggle_button_column_ = new_column;
      signal_change_();
    }
  }

  /// This might return a nullptr to indicate an unset control.
  theatre::SourceValue *GetSourceValue() const { return source_value_; }
  bool IsToggleButton() const { return is_toggle_button_; }
  bool NewToggleButtonColumn() const { return new_toggle_button_column_; }

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
  void onPresetValueDeleted();

  theatre::SourceValue *source_value_ = nullptr;
  bool is_toggle_button_ = false;
  bool new_toggle_button_column_ = false;
  bool display_name_ = true;
  bool display_flash_button_ = true;
  bool display_check_button_ = true;
  bool overlay_fade_buttons_ = true;
  sigc::signal<void()> signal_change_;
  sigc::connection source_value_deleted_connection_;
};

}  // namespace glight::gui

#endif
