#ifndef GUI_CREATE_CHASE_DIALOG_H_
#define GUI_CREATE_CHASE_DIALOG_H_

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/dialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

#include "theatre/forwards.h"

#include "gui/recursionlock.h"
#include "gui/components/nameframe.h"
#include "gui/components/objectbrowser.h"

namespace glight::gui {

class MainWindow;

class CreateChaseDialog : public Gtk::Dialog {
 public:
  CreateChaseDialog();

  sigc::signal<void(theatre::Chase &)> SignalNewChase() {
    return signal_new_chase_;
  }

 private:
  void initListPart();
  void initNewSequencePart();

  void onAddObjectToChaseButtonClicked();
  void onClearSequenceButtonClicked();
  void onCreateChaseButtonClicked();
  void onSelectedObjectChanged();

  Gtk::Paned _paned;

  Gtk::Frame _listFrame;
  ObjectBrowser _list;

  Gtk::TreeView _newChaseListView;
  Glib::RefPtr<Gtk::ListStore> _newChaseListModel;
  struct NewSequenceListColumns : public Gtk::TreeModelColumnRecord {
    NewSequenceListColumns() {
      add(_title);
      add(_controllable);
    }

    Gtk::TreeModelColumn<Glib::ustring> _title;
    Gtk::TreeModelColumn<theatre::Controllable *> _controllable;
  } _newChaseListColumns;

  Gtk::Box _listVBox{Gtk::Orientation::VERTICAL};
  Gtk::Box _newChaseBox;

  Gtk::Frame _newChaseFrame;

  Gtk::ScrolledWindow _newChaseScrolledWindow;

  Gtk::Box _newChaseButtonBox;
  Gtk::Button _addObjectToChaseButton, _clearChaseButton;

  Gtk::Button *_makeChaseButton;

  RecursionLock _delayUpdates;
  sigc::signal<void(theatre::Chase &)> signal_new_chase_;
};

}  // namespace glight::gui

#endif
