#ifndef CHASE_WIZARD_H
#define CHASE_WIZARD_H

#include "components/colorsequencewidget.h"
#include "components/objectbrowser.h"

#include "../theatre/autodesign.h"

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

class DesignWizard : public Gtk::Window {
 public:
  DesignWizard(class Management &management, class EventTransmitter &hub,
               const std::string &destinationPath);
  ~DesignWizard();

  void SetDestinationPath(const std::string &destinationPath) {
    _destinationPath = destinationPath;
  }

  void Select(const std::vector<class Fixture *> &fixtures);

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
  void onManagementChange(class Management &newManagement) {
    _management = &newManagement;
    fillFixturesList();
  }
  void onNextClicked();
  void initPage1();
  void initPage2();
  void initPage3_1RunningLight();
  void initPage3_2SingleColor();
  void initPage3_3ShiftColors();
  void initPage3_4VUMeter();
  void initPage3_5ColorPreset();
  void initPage3_6Increasing();
  class Folder &getFolder() const;

  // 1b
  void onAddControllable();
  void addControllable(FolderObject &object);
  void onRemoveControllable();
  void onControllableSelected();

  AutoDesign::ColorDeduction colorDeduction() const;

  class EventTransmitter &_eventHub;
  class Management *_management;
  std::string _destinationPath;

  Gtk::VBox _mainBox;
  Gtk::VBox _vBoxPage1, _vBoxPage1a, _vBoxPage1b, _vBoxPage2, _vBoxPage2Type,
      _vBoxPage2Deduction, _vBoxPage3_1, _vBoxPage3_2, _vBoxPage3_3,
      _vBoxPage3_4, _vBoxPage3_5, _vBoxPage3_6;
  Gtk::Notebook _notebook;
  // 1a
  Gtk::Label _selectLabel;
  Gtk::TreeView _fixturesListView;
  std::vector<class Controllable *> _selectedControllables;
  // 1b
  ObjectBrowser _objectBrowser;
  Gtk::HBox _controllableButtonBox;
  Gtk::Button _addControllableButton, _removeControllableButton;
  Gtk::TreeView _controllablesListView;

  Gtk::Frame _typeFrameP2, _deductionFrameP2;
  Gtk::RadioButton _colorPresetBtn, _runningLightBtn, _singleColorBtn,
      _shiftColorsBtn, _increaseBtn, _vuMeterBtn;
  Gtk::CheckButton _deduceWhite, _deduceAmber, _deduceUV;

  ColorSequenceWidget _colorsWidgetP3_1;
  Gtk::RadioButton _increasingRunRB, _decreasingRunRB, _backAndForthRunRB,
      _inwardRunRB, _outwardRunRB, _randomRunRB;

  ColorSequenceWidget _colorsWidgetP3_2;
  Gtk::Label _variationLabel;
  Gtk::Scale _variation;

  ColorSequenceWidget _colorsWidgetP3_3;
  Gtk::RadioButton _shiftIncreasingRB, _shiftDecreasingRB, _shiftBackAndForthRB,
      _shiftRandomRB;

  ColorSequenceWidget _colorsWidgetP3_4;
  Gtk::RadioButton _vuIncreasingRB, _vuDecreasingRB, _vuInwardRunRB,
      _vuOutwardRunRB;

  ColorSequenceWidget _colorsWidgetP3_5;
  Gtk::CheckButton _eachFixtureSeparatelyCB;

  ColorSequenceWidget _colorsWidgetP3_6;
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
    Gtk::TreeModelColumn<class Fixture *> _fixture;
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
    Gtk::TreeModelColumn<class Controllable *> _controllable;
  } _controllablesListColumns;
  Gtk::ScrolledWindow _controllablesScrolledWindow;
};

#endif  // CHASE_WIZARD_H
