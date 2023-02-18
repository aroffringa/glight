#ifndef GUI_CONTROLWIDGET_H_
#define GUI_CONTROLWIDGET_H_

#include <memory>

#include <gtkmm/bin.h>

#include "../../theatre/forwards.h"

namespace glight::gui {

class EventTransmitter;
class FaderWindow;

enum class ControlMode { Primary, Secondary };

/**
 * @author Andre Offringa
 * Base class for GUI controls that allow switching presets, and that
 * should obey fading or solo settings.
 */
class ControlWidget : public Gtk::Bin {
 public:
  ControlWidget(FaderWindow& fader_window,
                ControlMode mode);
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
   * Turn this fader fully on. This is for example
   * used by the parent window when a key is pressed
   * to turn on the fader.
   */
  virtual void FullOn() = 0;

  /**
   * Turn this fader off.
   */
  virtual void FullOff() = 0;

  /**
   * Link this control to the given source. If moveFader is true,
   * the control will change its state to reflect the value of the
   * source's value.
   */
  void Assign(theatre::SourceValue *item, bool moveFader);

  /**
   * Resyncs the fader with the source value.
   */
  virtual void MoveSlider() = 0;
  virtual void Limit(double value) = 0;

  sigc::signal<void(double)> &SignalValueChange() { return _signalValueChange; }
  sigc::signal<void> &SignalAssigned() { return _signalAssigned; }
  sigc::signal<void> &SignalDisplayChanged() { return _signalDisplayChanged; }

  theatre::SourceValue *GetSourceValue() const { return _sourceValue; }
  void Unassign() { Assign(nullptr, true); }
  void SetFadeUpSpeed(double fadePerSecond) { _fadeUpSpeed = fadePerSecond; }
  void SetFadeDownSpeed(double fadePerSecond) {
    _fadeDownSpeed = fadePerSecond;
  }
  ControlMode GetMode() const { return _mode; }

  static double MAX_SCALE_VALUE();

 protected:
  virtual void OnAssigned(bool moveFader) = 0;

  /**
   * Set the value after a fader change. This function
   * begins a fade if the corresponding fade up/down speed
   * are set.
   */
  void setTargetValue(unsigned target);

  /**
   * Sets the value, skipping any requested fade.
   */
  void setImmediateValue(unsigned value);

  EventTransmitter &GetEventHub() { return _eventHub; }
  theatre::Management &GetManagement() { return _management; }
  theatre::SingleSourceValue &GetSingleSourceValue();
  FaderWindow& GetFaderWindow() { return fader_window_; }

 private:
  void OnTheatreUpdate();

  double _fadeUpSpeed, _fadeDownSpeed;
  ControlMode _mode;
  theatre::SourceValue *_sourceValue = nullptr;
  FaderWindow& fader_window_;
  theatre::Management &_management;
  EventTransmitter &_eventHub;
  sigc::connection _updateConnection;
  sigc::signal<void(double)> _signalValueChange;
  sigc::signal<void> _signalAssigned;
  sigc::signal<void> _signalDisplayChanged;
};

}  // namespace glight::gui

#endif
