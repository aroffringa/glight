#ifndef GUI_CONTROLWIDGET_H_
#define GUI_CONTROLWIDGET_H_

#include <memory>

#include <gtkmm/bin.h>

#include "../../theatre/forwards.h"

namespace glight::gui {

class EventTransmitter;
class FaderState;
class FaderWindow;

enum class ControlMode { Primary, Secondary };

/**
 * @author Andre Offringa
 * Base class for GUI controls that allow switching presets, and that
 * should obey fading or solo settings.
 */
class ControlWidget : public Gtk::Bin {
 public:
  ControlWidget(FaderWindow &fader_window, FaderState &state, ControlMode mode);
  ~ControlWidget();

  /**
   * Toggle this fader. When the fader is off, it should
   * turn fully on, and when it is not off, it is turned
   * off. This is for example
   * used by the parent window when a key is pressed
   * to turn on the fader.
   */
  virtual void Toggle() = 0;

  /**
   * Turn this fader on. This is for example
   * used by the parent window when a key is pressed
   * to turn on the fader.
   */
  virtual void FlashOn() = 0;

  /**
   * Turn this fader off.
   */
  virtual void FlashOff() = 0;

  /**
   * Link this control to the given sources. If sync_fader is true,
   * the control will change its state to reflect the value of the
   * source's value. Otherwise, the source value will be set to the
   * value of the fader.
   */
  void Assign(const std::vector<theatre::SourceValue *> &sources,
              bool sync_fader);

  bool IsAssigned() const;

  /**
   * Resyncs the control with the source value.
   */
  virtual void SyncFader() = 0;
  virtual void Limit(double value) = 0;

  sigc::signal<void> &SignalValueChange() { return _signalValueChange; }
  sigc::signal<void> &SignalAssigned() { return _signalAssigned; }

  const std::vector<theatre::SourceValue *> &GetSourceValues() const {
    return sources_;
  }
  theatre::SourceValue *GetSourceValue(size_t index) const {
    return index < sources_.size() ? sources_[index] : nullptr;
  }
  void Unassign() { Assign({}, true); }
  void SetFadeUpSpeed(double fadePerSecond) { _fadeUpSpeed = fadePerSecond; }
  void SetFadeDownSpeed(double fadePerSecond) {
    _fadeDownSpeed = fadePerSecond;
  }
  ControlMode GetMode() const { return _mode; }

  /**
   * Number of source values that this controlwidget naturally
   * connects too. For a single slider or single check button
   * this is 1, but for choice or color selection wiget, this
   * may be different.
   * This does not need to match the number of assigned source values.
   * It is used for example for automatically assigned the faders to
   * unassigned source values.
   */
  size_t DefaultSourceCount() const { return default_source_count_; }

  static double MAX_SCALE_VALUE();

 protected:
  virtual void OnAssigned(bool moveFader) = 0;
  void ShowAssignDialog();

  /**
   * Sub-classes can call this to set the default source count
   * number. If not set, it defaults to 1.
   */
  void SetDefaultSourceCount(size_t default_source_count) {
    default_source_count_ = default_source_count;
  }

  /**
   * Set the value after a fader change. This function
   * begins a fade if the corresponding fade up/down speed
   * are set.
   */
  void setTargetValue(size_t source_index, unsigned target);

  /**
   * Sets the value, skipping any requested fade.
   */
  void setImmediateValue(size_t source_index, unsigned value);

  EventTransmitter &GetEventHub() { return _eventHub; }
  theatre::Management &GetManagement() { return _management; }
  theatre::SingleSourceValue &GetSingleSourceValue(size_t index) const;
  FaderWindow &GetFaderWindow() { return fader_window_; }

 protected:
  FaderState &State() { return _state; }

 private:
  void OnTheatreUpdate();

  double _fadeUpSpeed, _fadeDownSpeed;
  ControlMode _mode;
  FaderState &_state;
  size_t default_source_count_ = 1;
  std::vector<theatre::SourceValue *> sources_;
  FaderWindow &fader_window_;
  theatre::Management &_management;
  EventTransmitter &_eventHub;
  sigc::connection _updateConnection;
  sigc::signal<void> _signalValueChange;
  sigc::signal<void> _signalAssigned;
  sigc::signal<void> _signalDisplayChanged;
};

}  // namespace glight::gui

#endif
