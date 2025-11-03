#ifndef BEAT_INPUT_H
#define BEAT_INPUT_H

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../recursionlock.h"

namespace glight::gui {

class BeatInput : public Gtk::Box {
 public:
  BeatInput(double value);

  BeatInput(const std::string &label, double value);

  sigc::signal<void(double)> &SignalValueChanged() {
    return signal_value_changed_;
  }

  double Value() const { return ScaleToValue(scale_.get_value()); }

  void SetValue(double new_value);

 private:
  void Initialize(double value);

  static double ValueToScale(double value);
  static double ScaleToValue(size_t index);
  void OnScaleChanged();
  void SetValueLabel(unsigned index);

  Gtk::Label caption_label_;
  Gtk::Scale scale_;
  Gtk::Label value_label_;
  RecursionLock recursion_lock_;
  sigc::signal<void(double)> signal_value_changed_;
};

}  // namespace glight::gui

#endif
