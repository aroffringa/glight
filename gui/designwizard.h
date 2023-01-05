#ifndef GUI_CHASE_WIZARD_H_
#define GUI_CHASE_WIZARD_H_

#include "components/colorsequencewidget.h"
#include "components/fixturelist.h"
#include "components/foldercombo.h"
#include "components/objectbrowser.h"
#include "components/reorderwidget.h"

#include "../theatre/autodesign.h"
#include "../theatre/forwards.h"

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/notebook.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include <vector>

namespace glight::gui {

class EventTransmitter;

class DesignWizard : public Gtk::Window {
 public:
  DesignWizard(theatre::Management &management, EventTransmitter &hub);
  ~DesignWizard();

  void SetCurrentPath(const std::string &currentPath) {
    _currentPath = currentPath;
  }

  void Select(const std::vector<theatre::Fixture *> &fixtures) {
    _fixtureList.Select(fixtures);
  }

 private:
  enum Page {
    Page1_SelFixtures,
    Page2_Order,
    Page3_SelType,
    Page4_1_RunningLight,
    Page4_2_SingleColor,
    Page4_3_ShiftingColors,
    Page4_4_VUMeter,
    Page4_5_ColorPreset,
    Page4_6_Increasing
  };

  void onNextClicked();
  void initPage1();
  void initPage2();
  void initPage4_1RunningLight();
  void initPage4_2SingleColor();
  void initPage4_3ShiftColors();
  void initPage4_4VUMeter();
  void initPage4_5ColorPreset();
  void initPage4_6Increasing();
  void initPage4Destination(const std::string &name);
  theatre::Folder &getCurrentFolder() const;
  theatre::Folder &makeDestinationFolder() const;

  // 1b
  void onAddControllable();
  void addControllable(theatre::FolderObject &object);
  void onRemoveControllable();
  void onControllableSelected();

  theatre::AutoDesign::ColorDeduction colorDeduction() const;

  EventTransmitter &_eventHub;
  theatre::Management &_management;
  std::string _currentPath;

  Gtk::VBox _mainBox;
  Gtk::VBox _vBoxPage1, _vBoxPage1a, _vBoxPage1b;
  Gtk::Notebook _notebook;
  std::vector<theatre::Controllable *> _selectedControllables;
  // 1a
  Gtk::Label _selectLabel;
  components::FixtureList _fixtureList;
  // 1b
  ObjectBrowser _objectBrowser;
  Gtk::HBox _controllableButtonBox;
  Gtk::Button _addControllableButton, _removeControllableButton;
  Gtk::TreeView _controllablesListView;

  // 2 (order)
  components::ReorderWidget _reorderWidget;

  // 3
  Gtk::VBox _vBoxPage3, _vBoxPage3Type, _vBoxPage3Deduction;
  Gtk::Frame _typeFrameP3, _deductionFrameP3;
  Gtk::RadioButton _colorPresetBtn, _runningLightBtn, _singleColorBtn,
      _shiftColorsBtn, _increaseBtn, _vuMeterBtn;
  Gtk::CheckButton _deduceWhite, _deduceAmber, _deduceUV, _deduceLime;

  // Page 4 common widgets
  Gtk::VBox _vBoxPage4;
  ColorSequenceWidget _colorsWidgetP4;
  Gtk::Label _parentLabel;
  FolderCombo _parentFolderCombo;
  Gtk::HBox _folderNameBox;
  Gtk::CheckButton _newFolderCB;
  Gtk::Entry _newFolderNameEntry;

  // 4_1
  Gtk::RadioButton _increasingRunRB, _decreasingRunRB, _backAndForthRunRB,
      _inwardRunRB, _outwardRunRB, _randomRunRB;

  // 4_2
  Gtk::Label _variationLabel;
  Gtk::Scale _variation;

  // 4_3
  Gtk::RadioButton _shiftIncreasingRB, _shiftDecreasingRB, _shiftBackAndForthRB,
      _shiftRandomRB;

  // 4_4
  Gtk::RadioButton _vuIncreasingRB, _vuDecreasingRB, _vuInwardRunRB,
      _vuOutwardRunRB;

  // 4_5
  Gtk::CheckButton _eachFixtureSeparatelyCB;

  // 4_6
  Gtk::RadioButton _incForwardRB, _incBackwardRB, _incForwardReturnRB,
      _incBackwardReturnRB;

  Gtk::ButtonBox _buttonBox;
  Gtk::Button _nextButton;
  Page _currentPage;

  Glib::RefPtr<Gtk::ListStore> _controllablesListModel;
  struct ControllablesListColumns : public Gtk::TreeModelColumnRecord {
    ControllablesListColumns() {
      add(_title);
      add(_path);
      add(_controllable);
    }

    Gtk::TreeModelColumn<Glib::ustring> _title, _path;
    Gtk::TreeModelColumn<theatre::Controllable *> _controllable;
  } _controllablesListColumns;
  Gtk::ScrolledWindow _controllablesScrolledWindow;
};

}  // namespace glight::gui

#endif  // CHASE_WIZARD_H
