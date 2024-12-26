#ifndef GLIGHT_GUI_POWER_MONITOR_H_
#define GLIGHT_GUI_POWER_MONITOR_H_

#include <vector>

#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/progressbar.h>

#include "theatre/valuesnapshot.h"

#include "gui/scopedconnection.h"

namespace glight::gui::components {

class PowerMonitor : public Gtk::Grid {
 public:
  PowerMonitor();
  void Start();

 private:
  struct Row {
    Gtk::Label label_{"- / - KW"};
    Gtk::ProgressBar progress_bar_;
  };
  void Update();
  void TimeUpdate();
  void SetRow(size_t row_index, double used_power, double max_power);

  ScopedConnection timeout_connection_;
  ScopedConnection update_connection_;
  std::vector<Row> rows_;
  theatre::ValueSnapshot snapshot_;

 private:
  void UpdateValues();
};

}  // namespace glight::gui::components

#endif
