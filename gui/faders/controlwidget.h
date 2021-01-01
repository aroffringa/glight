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
  ControlWidget() {}

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
   * the control will change its state to reflact the value of the
   * source's value.
   */
  virtual void Assign(class SourceValue *item, bool moveFader) = 0;

  /**
   * Resyncs the fader with the preset value.
   */
  virtual void MoveSlider() = 0;
  virtual class SourceValue *GetSourceValue() const = 0;
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

 protected:
  double _fadeUpSpeed, _fadeDownSpeed;

  /**
   * Set the value after a fader change. This function
   * begins a fade if the corresponding fade up/down speed
   * are set.
   */
  void setValue(unsigned target);

  /**
   * Sets the value, skipping any requested fade.
   */
  void setImmediateValue(unsigned target);

 private:
  sigc::signal<void, double> _signalValueChange;
  sigc::signal<void> _signalAssigned;
};

#endif
