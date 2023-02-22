#ifndef DURATION_INPUT_H
#define DURATION_INPUT_H

#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/scale.h>

#include "../recursionlock.h"

namespace glight::gui {

class DurationInput : public Gtk::HBox {
 public:
  DurationInput(double value);

  DurationInput(const std::string &label, double value);

  sigc::signal<void(double)> &SignalValueChanged() {
    return signal_value_changed_;
  }

  double Value() const { return atof(entry_.get_text().c_str()) * 1e3; }

  void SetValue(double newValue);

 private:
  void Initialize(double value);

  static double ValueToScale(double value);
  void onScaleChanged();
  void OnEntryChanged();
  void SetEntry(double value);

  Gtk::Label label_;
  Gtk::HScale scale_;
  Gtk::Entry entry_;
  RecursionLock recursion_lock_;
  sigc::signal<void(double)> signal_value_changed_;
};

}  // namespace glight::gui

#endif
