#ifndef GUI_FADERS_COMBO_CONTROL_WIDGET_H_
#define GUI_FADERS_COMBO_CONTROL_WIDGET_H_

#include "controlwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/combobox.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>

namespace glight::gui {

class ComboControlWidget final : public ControlWidget {
 public:
  ComboControlWidget(FaderWindow &fader_window, FaderState &state,
                     ControlMode mode, char key);
  ~ComboControlWidget();

  virtual void Toggle() override;
  virtual void FlashOn() override;
  virtual void FlashOff() override;
  virtual void SyncFader() override;

  virtual void Limit(double value) override;

  theatre::SourceValue *SelectedSource() const;

 private:
  Gtk::ListStore::iterator FirstNonZeroValue() const;
  virtual void OnAssigned(bool moveFader) override;
  void OnChanged();
  void UpdateDisplaySettings();
  bool HandleRightRelease(GdkEventButton *event);
  void OpenDescriptionDialog();

  Gtk::VBox box_;
  Gtk::EventBox event_box_;
  Gtk::Label description_label_{"<No description>"};
  Glib::RefPtr<Gtk::ListStore> model_;
  struct ListColumns : public Gtk::TreeModelColumnRecord {
    ListColumns() {
      add(title_);
      add(source_);
    }
    Gtk::TreeModelColumn<Glib::ustring> title_;
    Gtk::TreeModelColumn<theatre::SourceValue *> source_;
  } columns_;
  Gtk::ComboBox combo_;

  bool hold_updates_ = false;

  sigc::connection update_display_settings_connection_;
};

}  // namespace glight::gui

#endif
