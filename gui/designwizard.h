#ifndef GUI_CHASE_WIZARD_H_
#define GUI_CHASE_WIZARD_H_

#include "components/colorsequencewidget.h"
#include "components/foldercombo.h"
#include "components/objectbrowser.h"

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
  DesignWizard(theatre::Management &management, EventTransmitter &hub,
               const std::string &currentPath);
  ~DesignWizard();

  void SetCurrentPath(const std::string &currentPath) {
    _currentPath = currentPath;
  }

  void Select(const std::vector<theatre::Fixture *> &fixtures);

 private:
  enum Page {
    Page1_SelFixtures,
    Page2_SelType,
    Page3_1_RunningLight,
    Page3_2_SingleColor,
    Page3_3_ShiftingColors,
    Page3_4_VUMeter,
    Page3_5_ColorPreset,
    Page3_6_Increasing
  };

  void fillFixturesList();
  void onNextClicked();
  void initPage1();
  void initPage2();
  void initPage3_1RunningLight();
  void initPage3_2SingleColor();
  void initPage3_3ShiftColors();
  void initPage3_4VUMeter();
  void initPage3_5ColorPreset();
  void initPage3_6Increasing();
  void initPage3Destination(const std::string &name);
  theatre::Folder &getCurrentFolder() const;
  theatre::Folder &makeDestinationFolder() const;

  // 1b
  void onAddControllable();
  void addControllable(theatre::FolderObject &object);
  void onRemoveControllable();
  void onControllableSelected();

  theatre::AutoDesign::ColorDeduction colorDeduction() const;

  EventTransmitter &_eventHub;
  theatre::Management *_management;
  std::string _currentPath;

  Gtk::VBox _mainBox;
  Gtk::VBox _vBoxPage1, _vBoxPage1a, _vBoxPage1b, _vBoxPage2, _vBoxPage2Type,
      _vBoxPage2Deduction;
  Gtk::Notebook _notebook;
  // 1a
  Gtk::Label _selectLabel;
  Gtk::TreeView _fixturesListView;
  std::vector<theatre::Controllable *> _selectedControllables;
  // 1b
  ObjectBrowser _objectBrowser;
  Gtk::HBox _controllableButtonBox;
  Gtk::Button _addControllableButton, _removeControllableButton;
  Gtk::TreeView _controllablesListView;

  Gtk::Frame _typeFrameP2, _deductionFrameP2;
  Gtk::RadioButton _colorPresetBtn, _runningLightBtn, _singleColorBtn,
      _shiftColorsBtn, _increaseBtn, _vuMeterBtn;
  Gtk::CheckButton _deduceWhite, _deduceAmber, _deduceUV, _deduceLime;

  // Page 3 common widgets
  Gtk::VBox _vBoxPage3;
  ColorSequenceWidget _colorsWidgetP3;
  Gtk::Label _parentLabel;
  FolderCombo _parentFolderCombo;
  Gtk::HBox _folderNameBox;
  Gtk::CheckButton _newFolderCB;
  Gtk::Entry _newFolderNameEntry;

  // 3_1
  Gtk::RadioButton _increasingRunRB, _decreasingRunRB, _backAndForthRunRB,
      _inwardRunRB, _outwardRunRB, _randomRunRB;

  // 3_2
  Gtk::Label _variationLabel;
  Gtk::Scale _variation;

  // 3_3
  Gtk::RadioButton _shiftIncreasingRB, _shiftDecreasingRB, _shiftBackAndForthRB,
      _shiftRandomRB;

  // 3_4
  Gtk::RadioButton _vuIncreasingRB, _vuDecreasingRB, _vuInwardRunRB,
      _vuOutwardRunRB;

  // 3_5
  Gtk::CheckButton _eachFixtureSeparatelyCB;

  // 3_6
  Gtk::RadioButton _incForwardRB, _incBackwardRB, _incForwardReturnRB,
      _incBackwardReturnRB;

  Gtk::ButtonBox _buttonBox;
  Gtk::Button _nextButton;
  Page _currentPage;

  Glib::RefPtr<Gtk::ListStore> _fixturesListModel;
  struct FixturesListColumns : public Gtk::TreeModelColumnRecord {
    FixturesListColumns() {
      add(_title);
      add(_type);
      add(_fixture);
    }

    Gtk::TreeModelColumn<Glib::ustring> _title, _type;
    Gtk::TreeModelColumn<theatre::Fixture *> _fixture;
  } _fixturesListColumns;
  Gtk::ScrolledWindow _fixturesScrolledWindow;

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
