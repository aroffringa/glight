#ifndef CONTROLWIDGET_H
#define CONTROLWIDGET_H

#include <memory>

#include <gtkmm/bin.h>

/**
 * @author Andre Offringa
 * Base class for GUI controls that allow switching presets, and that
 * should obey fading or solo settings.
 */
class ControlWidget : public Gtk::Bin {
 public:
  ControlWidget() : _fadingValue(0), _targetValue(0) {}

  virtual void Toggle() = 0;
  virtual void FullOn() = 0;
  virtual void FullOff() = 0;
  /**
   * Link this control to the given preset. If moveFader is true,
   * the control will change its state to reflact the value of the
   * preset.
   */
  virtual void Assign(class PresetValue *item, bool moveFader) = 0;
  virtual void MoveSlider() = 0;
  virtual class PresetValue *Preset() const = 0;
  virtual void Limit(double value) = 0;
  virtual void ChangeManagement(class Management &management,
                                bool moveSliders) = 0;

  sigc::signal<void, double> &SignalValueChange() { return _signalValueChange; }
  sigc::signal<void> &SignalAssigned() { return _signalAssigned; }

  void Unassign() { Assign(nullptr, false); }
  void SetFadeUpSpeed(double fadePerSecond) { _fadeUpSpeed = fadePerSecond; }
  void SetFadeDownSpeed(double fadePerSecond) {
    _fadeDownSpeed = fadePerSecond;
  }

  static double MAX_SCALE_VALUE();

  void writeValue(unsigned target);
  void UpdateValue(double timePassed);

 protected:
  double _fadeUpSpeed, _fadeDownSpeed;

  void setImmediate(unsigned value) {
    _fadingValue = value;
    _targetValue = value;
  }

 private:
  unsigned _fadingValue, _targetValue;

  sigc::signal<void, double> _signalValueChange;
  sigc::signal<void> _signalAssigned;
};

#endif
