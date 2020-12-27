#ifndef CREATE_CHASE_DIALOG_H
#define CREATE_CHASE_DIALOG_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/dialog.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

#include "nameframe.h"
#include "recursionlock.h"

#include "components/objectbrowser.h"

/**
        @author Andre Offringa
*/
class CreateChaseDialog : public Gtk::Dialog {
public:
  CreateChaseDialog(class Management &management,
                    class ShowWindow &parentWindow);

  class Chase &CreatedChase() {
    return *_newChase;
  }

private:
  void initListPart();
  void initNewSequencePart();

  void onAddObjectToChaseButtonClicked();
  void onClearSequenceButtonClicked();
  void onCreateChaseButtonClicked();
  void onSelectedObjectChanged();

  void changeManagement(class Management &management) {
    _management = &management;
  }

  Gtk::VPaned _paned;

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
    Gtk::TreeModelColumn<class Controllable *> _controllable;
  } _newChaseListColumns;

  Gtk::VBox _listVBox;
  Gtk::HBox _newChaseBox;

  Gtk::Frame _newChaseFrame;

  Gtk::ScrolledWindow _newChaseScrolledWindow;

  Gtk::VButtonBox _newChaseButtonBox;
  Gtk::Button _addObjectToChaseButton, _clearChaseButton;

  Gtk::Button *_makeChaseButton;

  Management *_management;
  class ShowWindow &_parentWindow;
  RecursionLock _delayUpdates;
  class Chase *_newChase;
};

#endif
