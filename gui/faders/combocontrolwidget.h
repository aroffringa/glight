#ifndef GUI_FADERS_COMBO_CONTROL_WIDGET_H_
#define GUI_FADERS_COMBO_CONTROL_WIDGET_H_

#include "controlwidget.h"

#include <gtkmm/box.h>
#include <gtkmm/combobox.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>

namespace glight::gui {

class ComboControlWidget final : public ControlWidget {
 public:
  ComboControlWidget(FaderWindow &fader_window, FaderState &state,
                     ControlMode mode, char key);
  ~ComboControlWidget();

  virtual void Toggle() final;
  virtual void FlashOn() final;
  virtual void FlashOff() final;
  virtual void SyncFader() final;

  virtual void Limit(double value) final;

  theatre::SourceValue *SelectedSource() const;

 private:
  Gtk::ListStore::iterator FirstNonZeroValue() const;
  virtual void OnAssigned(bool moveFader) final;
  void PrepareContextMenu(ControlMenu &menu) final;
  void OnChanged();
  void UpdateDisplaySettings();
  void HandleRightRelease();
  void OpenDescriptionDialog();

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
  std::unique_ptr<Gtk::Dialog> dialog_;
};

}  // namespace glight::gui

#endif
