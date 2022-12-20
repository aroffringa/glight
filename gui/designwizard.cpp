#include "designwizard.h"
#include "eventtransmitter.h"

#include "components/colorselectwidget.h"

#include "../theatre/fixture.h"
#include "../theatre/fixturecontrol.h"
#include "../theatre/folder.h"
#include "../theatre/folderoperations.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"

#include <memory>

namespace glight::gui {

using theatre::AutoDesign;

DesignWizard::DesignWizard(theatre::Management &management,
                           EventTransmitter &hub,
                           const std::string &currentPath)
    : _eventHub(hub),
      _management(&management),
      _currentPath(currentPath),
      _selectLabel("Select fixtures:"),
      _objectBrowser(management, hub),

      _typeFrameP2("Type"),
      _deductionFrameP2("Colour deduction"),
      _colorPresetBtn("Colour preset"),
      _runningLightBtn("Running light"),
      _singleColorBtn("Random around single colour"),
      _shiftColorsBtn("Shifting colours"),
      _increaseBtn("Increasing colours"),
      _vuMeterBtn("VU meter"),
      _deduceWhite("White from RGB"),
      _deduceAmber("Amber from RGB"),
      _deduceUV("UV from RGB"),
      _deduceLime("Lime from RGB"),

      _colorsWidgetP3(this),
      _parentFolderCombo(management, hub),
      _newFolderCB("New folder: "),
      _increasingRunRB("Increasing order"),
      _decreasingRunRB("Decreasing order"),
      _backAndForthRunRB("Back and forth"),
      _inwardRunRB("Inward"),
      _outwardRunRB("Outward"),
      _randomRunRB("Randomized"),
      _variationLabel("Variation:"),
      _shiftIncreasingRB("Increasing shift"),
      _shiftDecreasingRB("Decreasing shift"),
      _shiftBackAndForthRB("Back and forth"),
      _shiftRandomRB("Move randomly"),
      _vuIncreasingRB("Increasing direction"),
      _vuDecreasingRB("Decreasing direction"),
      _vuInwardRunRB("Inward direction"),
      _vuOutwardRunRB("Outward direcion"),
      _eachFixtureSeparatelyCB("One preset per fixture"),
      _incForwardRB("Forward direction"),
      _incBackwardRB("Backward direction"),
      _incForwardReturnRB("Forward and return"),
      _incBackwardReturnRB("Backward and return"),
      _nextButton("Next"),
      _currentPage(Page1_SelFixtures) {
  initPage1();
  initPage2();

  _mainBox.pack_start(_vBoxPage1, true, true);
  _vBoxPage1.show_all();

  _nextButton.signal_clicked().connect(
      sigc::mem_fun(*this, &DesignWizard::onNextClicked));
  _buttonBox.pack_start(_nextButton);
  _buttonBox.show_all();
  _mainBox.pack_end(_buttonBox, false, false);

  add(_mainBox);
  _mainBox.show();
}

DesignWizard::~DesignWizard() {}

void DesignWizard::initPage1() {
  _vBoxPage1.pack_start(_notebook);

  _notebook.append_page(_vBoxPage1a, "Fixtures");
  _vBoxPage1a.pack_start(_selectLabel);

  _fixturesListModel = Gtk::ListStore::create(_fixturesListColumns);
  _fixturesListView.set_model(_fixturesListModel);
  _fixturesListView.append_column("Fixture", _fixturesListColumns._title);
  _fixturesListView.append_column("Type", _fixturesListColumns._type);
  _fixturesListView.set_rubber_banding(true);
  _fixturesListView.get_selection()->set_mode(
      Gtk::SelectionMode::SELECTION_MULTIPLE);
  fillFixturesList();
  _fixturesScrolledWindow.add(_fixturesListView);
  _fixturesScrolledWindow.set_size_request(300, 400);
  _vBoxPage1a.pack_start(_fixturesScrolledWindow);

  _notebook.append_page(_vBoxPage1b, "Any controllables");

  _objectBrowser.SignalSelectionChange().connect(
      [&]() { onControllableSelected(); });
  _objectBrowser.SignalObjectActivated().connect(
      [&](theatre::FolderObject &object) { addControllable(object); });
  _vBoxPage1b.pack_start(_objectBrowser);

  _addControllableButton.set_image_from_icon_name("go-down");
  _addControllableButton.set_sensitive(false);
  _addControllableButton.signal_clicked().connect(
      sigc::mem_fun(*this, &DesignWizard::onAddControllable));
  _controllableButtonBox.pack_start(_addControllableButton, false, false, 4);

  _removeControllableButton.set_image_from_icon_name("go-up");
  _removeControllableButton.signal_clicked().connect(
      sigc::mem_fun(*this, &DesignWizard::onRemoveControllable));
  _controllableButtonBox.pack_start(_removeControllableButton, false, false, 4);

  _controllableButtonBox.set_halign(Gtk::ALIGN_CENTER);
  _vBoxPage1b.pack_start(_controllableButtonBox, false, false);

  _controllablesListModel = Gtk::ListStore::create(_controllablesListColumns);
  _controllablesListView.set_model(_controllablesListModel);
  _controllablesListView.append_column("Controllable",
                                       _fixturesListColumns._title);
  _controllablesListView.append_column("Path", _fixturesListColumns._type);
  _controllablesListView.set_rubber_banding(true);
  _controllablesListView.get_selection()->set_mode(
      Gtk::SelectionMode::SELECTION_MULTIPLE);
  _controllablesScrolledWindow.add(_controllablesListView);
  _vBoxPage1b.pack_end(_controllablesScrolledWindow, true, true);
}

void DesignWizard::initPage2() {
  Gtk::RadioButtonGroup group;
  _colorPresetBtn.set_group(group);
  _vBoxPage2Type.pack_start(_colorPresetBtn);
  _runningLightBtn.set_group(group);
  _vBoxPage2Type.pack_start(_runningLightBtn);
  _singleColorBtn.set_group(group);
  _vBoxPage2Type.pack_start(_singleColorBtn);
  _shiftColorsBtn.set_group(group);
  _vBoxPage2Type.pack_start(_shiftColorsBtn);
  _increaseBtn.set_group(group);
  _vBoxPage2Type.pack_start(_increaseBtn);
  _vuMeterBtn.set_group(group);
  _vBoxPage2Type.pack_start(_vuMeterBtn);

  _typeFrameP2.add(_vBoxPage2Type);
  _vBoxPage2.pack_start(_typeFrameP2);

  _deduceWhite.set_active(true);
  _vBoxPage2Deduction.pack_start(_deduceWhite);
  _deduceAmber.set_active(true);
  _vBoxPage2Deduction.pack_start(_deduceAmber);
  _deduceUV.set_active(true);
  _vBoxPage2Deduction.pack_start(_deduceUV);
  _deduceLime.set_active(true);
  _vBoxPage2Deduction.pack_start(_deduceLime);

  _deductionFrameP2.add(_vBoxPage2Deduction);
  _vBoxPage2.pack_start(_deductionFrameP2);
}

void DesignWizard::initPage3_1RunningLight() {
  initPage3Destination("Running light");
  _vBoxPage3.pack_start(_colorsWidgetP3, true, true);
  Gtk::RadioButtonGroup group;
  _increasingRunRB.set_group(group);
  _vBoxPage3.pack_start(_increasingRunRB, false, false);
  _decreasingRunRB.set_group(group);
  _vBoxPage3.pack_start(_decreasingRunRB, false, false);
  _backAndForthRunRB.set_group(group);
  _vBoxPage3.pack_start(_backAndForthRunRB, false, false);
  _inwardRunRB.set_group(group);
  _vBoxPage3.pack_start(_inwardRunRB, false, false);
  _outwardRunRB.set_group(group);
  _vBoxPage3.pack_start(_outwardRunRB, false, false);
  _randomRunRB.set_group(group);
  _vBoxPage3.pack_start(_randomRunRB, false, false);
}

void DesignWizard::initPage3_2SingleColor() {
  initPage3Destination("Same color run");
  _vBoxPage3.pack_start(_colorsWidgetP3, true, true);
  _vBoxPage3.pack_start(_variationLabel, false, false);
  _variation.set_range(0, 100);
  _variation.set_increments(1.0, 10.0);
  _vBoxPage3.pack_start(_variation, false, false);
}

void DesignWizard::initPage3_3ShiftColors() {
  initPage3Destination("Shift colors");
  _vBoxPage3.pack_start(_colorsWidgetP3, true, true);
  Gtk::RadioButtonGroup group;
  _shiftIncreasingRB.set_group(group);
  _vBoxPage3.pack_start(_shiftIncreasingRB, false, false);
  _shiftDecreasingRB.set_group(group);
  _vBoxPage3.pack_start(_shiftDecreasingRB, false, false);
  _shiftBackAndForthRB.set_group(group);
  _vBoxPage3.pack_start(_shiftBackAndForthRB, false, false);
  _shiftRandomRB.set_group(group);
  _vBoxPage3.pack_start(_shiftRandomRB, false, false);
}

void DesignWizard::initPage3_4VUMeter() {
  initPage3Destination("VUMeter");
  _vBoxPage3.pack_start(_colorsWidgetP3, true, true);
  Gtk::RadioButtonGroup group;
  _vuIncreasingRB.set_group(group);
  _vBoxPage3.pack_start(_vuIncreasingRB, false, false);
  _vuDecreasingRB.set_group(group);
  _vBoxPage3.pack_start(_vuDecreasingRB, false, false);
  _vuInwardRunRB.set_group(group);
  _vBoxPage3.pack_start(_vuInwardRunRB, false, false);
  _vuOutwardRunRB.set_group(group);
  _vBoxPage3.pack_start(_vuOutwardRunRB, false, false);
}

void DesignWizard::initPage3_5ColorPreset() {
  initPage3Destination("Color preset");
  _vBoxPage3.pack_start(_colorsWidgetP3, true, true);
  _vBoxPage3.pack_start(_eachFixtureSeparatelyCB, false, false);
}

void DesignWizard::initPage3_6Increasing() {
  initPage3Destination("Increase chase");
  _vBoxPage3.pack_start(_colorsWidgetP3, true, true);
  Gtk::RadioButtonGroup group;
  _incForwardRB.set_group(group);
  _vBoxPage3.pack_start(_incForwardRB, false, false);
  _incBackwardRB.set_group(group);
  _vBoxPage3.pack_start(_incBackwardRB, false, false);
  _incForwardReturnRB.set_group(group);
  _vBoxPage3.pack_start(_incForwardReturnRB, false, false);
  _incBackwardReturnRB.set_group(group);
  _vBoxPage3.pack_start(_incBackwardReturnRB, false, false);
}

void DesignWizard::initPage3Destination(const std::string &name) {
  _parentLabel.set_text("Destination:");
  _vBoxPage3.pack_start(_parentLabel, false, false);
  theatre::Folder &folder = getCurrentFolder();
  _parentFolderCombo.Select(folder);
  _vBoxPage3.pack_start(_parentFolderCombo, false, false);
  _newFolderCB.set_active(true);
  _folderNameBox.pack_start(_newFolderCB, false, false);
  const std::string new_name = folder.GetAvailableName(name);
  _newFolderNameEntry.set_text(new_name);
  _folderNameBox.pack_start(_newFolderNameEntry, false, false);
  _vBoxPage3.pack_start(_folderNameBox, false, false);
}

void DesignWizard::fillFixturesList() {
  _fixturesListModel->clear();

  std::lock_guard<std::mutex> lock(_management->Mutex());
  const std::vector<std::unique_ptr<theatre::Fixture>> &fixtures =
      _management->GetTheatre().Fixtures();
  for (const std::unique_ptr<theatre::Fixture> &fixture : fixtures) {
    Gtk::TreeModel::iterator iter = _fixturesListModel->append();
    Gtk::TreeModel::Row row = *iter;
    row[_fixturesListColumns._title] = fixture->Name();
    row[_fixturesListColumns._type] = fixture->Type().Name();
    row[_fixturesListColumns._fixture] = fixture.get();
  }
}

void DesignWizard::Select(const std::vector<theatre::Fixture *> &fixtures) {
  _fixturesListView.get_selection()->unselect_all();
  Gtk::TreeModel::iterator iter;
  Gtk::TreeModel::Children children = _fixturesListModel->children();
  for (auto &child : children) {
    const theatre::Fixture *fixture =
        child.get_value(_fixturesListColumns._fixture);
    const auto iter = std::find(fixtures.begin(), fixtures.end(), fixture);
    if (iter != fixtures.end())
      _fixturesListView.get_selection()->select(child);
  }
}

theatre::Folder &DesignWizard::getCurrentFolder() const {
  theatre::Folder *folder = dynamic_cast<theatre::Folder *>(
      _management->GetObjectFromPathIfExists(_currentPath));
  if (folder)
    return *folder;
  else
    return _management->RootFolder();
}

void DesignWizard::onNextClicked() {
  switch (_currentPage) {
    case Page1_SelFixtures: {
      _selectedControllables.clear();
      if (_notebook.get_current_page() == 0) {
        Glib::RefPtr<Gtk::TreeSelection> selection =
            _fixturesListView.get_selection();
        auto selected = selection->get_selected_rows();
        for (auto &row : selected) {
          theatre::Fixture *fixture = (*_fixturesListModel->get_iter(
              row))[_fixturesListColumns._fixture];
          _selectedControllables.emplace_back(
              &_management->GetFixtureControl(*fixture));
        }
      } else {
        for (auto &iter : _controllablesListModel->children()) {
          _selectedControllables.emplace_back(
              (*iter)[_controllablesListColumns._controllable]);
        }
      }
      _mainBox.remove(_vBoxPage1);
      _mainBox.pack_start(_vBoxPage2, true, true);
      _vBoxPage2.show_all();
      _currentPage = Page2_SelType;
    } break;

    case Page2_SelType:
      _mainBox.remove(_vBoxPage2);
      if (_colorPresetBtn.get_active()) {
        initPage3_5ColorPreset();
        _colorsWidgetP3.SetColors(
            std::vector<Color>(_selectedControllables.size(), Color::White()));
        _colorsWidgetP3.SetMinCount(_selectedControllables.size());
        _colorsWidgetP3.SetMaxCount(_selectedControllables.size());
        _mainBox.pack_start(_vBoxPage3, true, true);
        _vBoxPage3.show_all();
        _currentPage = Page3_5_ColorPreset;
      } else if (_runningLightBtn.get_active()) {
        initPage3_1RunningLight();
        _colorsWidgetP3.SetColors(
            std::vector<Color>(_selectedControllables.size(), Color::White()));
        _colorsWidgetP3.SetMaxCount(_selectedControllables.size());
        _mainBox.pack_start(_vBoxPage3, true, true);
        _vBoxPage3.show_all();
        _currentPage = Page3_1_RunningLight;
      } else if (_singleColorBtn.get_active()) {
        initPage3_2SingleColor();
        _mainBox.pack_start(_vBoxPage3, true, true);
        _vBoxPage3.show_all();
        _currentPage = Page3_2_SingleColor;
      } else if (_shiftColorsBtn.get_active()) {
        initPage3_3ShiftColors();
        _colorsWidgetP3.SetMinCount(1);
        _colorsWidgetP3.SetMaxCount(_selectedControllables.size());
        _colorsWidgetP3.SetColors(
            std::vector<Color>(_selectedControllables.size(), Color::White()));
        _mainBox.pack_start(_vBoxPage3, true, true);
        _vBoxPage3.show_all();
        _currentPage = Page3_3_ShiftingColors;
      } else if (_vuMeterBtn.get_active()) {
        initPage3_4VUMeter();
        _colorsWidgetP3.SetMinCount(_selectedControllables.size());
        _colorsWidgetP3.SetMaxCount(_selectedControllables.size());
        _mainBox.pack_start(_vBoxPage3, true, true);
        _vBoxPage3.show_all();
        _currentPage = Page3_4_VUMeter;
      } else {
        initPage3_6Increasing();
        _colorsWidgetP3.SetMinCount(_selectedControllables.size());
        _colorsWidgetP3.SetMaxCount(_selectedControllables.size());
        _mainBox.pack_start(_vBoxPage3, true, true);
        _vBoxPage3.show_all();
        _currentPage = Page3_6_Increasing;
      }
      break;

    case Page3_1_RunningLight: {
      enum AutoDesign::RunType runType;
      if (_increasingRunRB.get_active())
        runType = AutoDesign::IncreasingRun;
      else if (_decreasingRunRB.get_active())
        runType = AutoDesign::DecreasingRun;
      else if (_backAndForthRunRB.get_active())
        runType = AutoDesign::BackAndForthRun;
      else if (_inwardRunRB.get_active())
        runType = AutoDesign::InwardRun;
      else if (_outwardRunRB.get_active())
        runType = AutoDesign::OutwardRun;
      else  // if(_randomRunRB.get_active())
        runType = AutoDesign::RandomRun;
      AutoDesign::MakeRunningLight(
          *_management, makeDestinationFolder(), _selectedControllables,
          _colorsWidgetP3.GetColors(), colorDeduction(), runType);
      _eventHub.EmitUpdate();
      hide();
    } break;

    case Page3_2_SingleColor:
      AutoDesign::MakeColorVariation(*_management, makeDestinationFolder(),
                                     _selectedControllables,
                                     _colorsWidgetP3.GetColors(),
                                     colorDeduction(), _variation.get_value());
      _eventHub.EmitUpdate();
      hide();
      break;

    case Page3_3_ShiftingColors: {
      enum AutoDesign::ShiftType shiftType;
      if (_shiftIncreasingRB.get_active())
        shiftType = AutoDesign::IncreasingShift;
      else if (_shiftDecreasingRB.get_active())
        shiftType = AutoDesign::DecreasingShift;
      else if (_shiftBackAndForthRB.get_active())
        shiftType = AutoDesign::BackAndForthShift;
      else
        shiftType = AutoDesign::RandomShift;
      AutoDesign::MakeColorShift(
          *_management, makeDestinationFolder(), _selectedControllables,
          _colorsWidgetP3.GetColors(), colorDeduction(), shiftType);
      _eventHub.EmitUpdate();
      hide();
    } break;

    case Page3_4_VUMeter: {
      AutoDesign::VUMeterDirection direction;
      if (_vuIncreasingRB.get_active())
        direction = AutoDesign::VUIncreasing;
      else if (_vuDecreasingRB.get_active())
        direction = AutoDesign::VUDecreasing;
      else if (_vuInwardRunRB.get_active())
        direction = AutoDesign::VUInward;
      else  // if(_vuOutwardRunRB.get_active())
        direction = AutoDesign::VUOutward;
      AutoDesign::MakeVUMeter(
          *_management, makeDestinationFolder(), _selectedControllables,
          _colorsWidgetP3.GetColors(), colorDeduction(), direction);
      _eventHub.EmitUpdate();
      hide();
    } break;

    case Page3_5_ColorPreset: {
      if (_eachFixtureSeparatelyCB.get_active())
        AutoDesign::MakeColorPresetPerFixture(
            *_management, makeDestinationFolder(), _selectedControllables,
            _colorsWidgetP3.GetColors(), colorDeduction());
      else
        AutoDesign::MakeColorPreset(
            *_management, makeDestinationFolder(), _selectedControllables,
            _colorsWidgetP3.GetColors(), colorDeduction());
      _eventHub.EmitUpdate();
      hide();
    } break;

    case Page3_6_Increasing: {
      AutoDesign::IncreasingType incType;
      if (_incForwardRB.get_active())
        incType = AutoDesign::IncForward;
      else if (_incBackwardRB.get_active())
        incType = AutoDesign::IncBackward;
      else if (_incForwardReturnRB.get_active())
        incType = AutoDesign::IncForwardReturn;
      else  // if(_incBackwardReturnRB.get_active())
        incType = AutoDesign::IncBackwardReturn;
      AutoDesign::MakeIncreasingChase(
          *_management, makeDestinationFolder(), _selectedControllables,
          _colorsWidgetP3.GetColors(), colorDeduction(), incType);
      _eventHub.EmitUpdate();
      hide();
    } break;
  }
}

void DesignWizard::addControllable(theatre::FolderObject &object) {
  theatre::Controllable *controllable =
      dynamic_cast<theatre::Controllable *>(&object);
  if (controllable) {
    Gtk::TreeModel::iterator iter = _controllablesListModel->append();
    Gtk::TreeModel::Row row = *iter;
    if (iter) {
      row[_controllablesListColumns._controllable] = controllable;
      row[_controllablesListColumns._title] = controllable->Name();
      row[_controllablesListColumns._path] = controllable->Parent().FullPath();
    }
  }
}

void DesignWizard::onAddControllable() {
  theatre::FolderObject *object = _objectBrowser.SelectedObject();
  if (object) addControllable(*object);
}

void DesignWizard::onRemoveControllable() {
  std::vector<Gtk::TreeModel::Path> iter =
      _controllablesListView.get_selection()->get_selected_rows();

  for (std::vector<Gtk::TreeModel::Path>::reverse_iterator elementIter =
           iter.rbegin();
       elementIter != iter.rend(); ++elementIter)
    _controllablesListModel->erase(
        _controllablesListModel->get_iter(*elementIter));
}

void DesignWizard::onControllableSelected() {
  theatre::Controllable *object =
      dynamic_cast<theatre::Controllable *>(_objectBrowser.SelectedObject());
  _addControllableButton.set_sensitive(object != nullptr);
}

AutoDesign::ColorDeduction DesignWizard::colorDeduction() const {
  AutoDesign::ColorDeduction deduction;
  deduction.whiteFromRGB = _deduceWhite.get_active();
  deduction.amberFromRGB = _deduceAmber.get_active();
  deduction.uvFromRGB = _deduceUV.get_active();
  deduction.limeFromRGB = _deduceLime.get_active();
  return deduction;
}

theatre::Folder &DesignWizard::makeDestinationFolder() const {
  theatre::Folder *folder = &_parentFolderCombo.Selection();
  if (_newFolderCB.get_active()) {
    const std::string new_name = _newFolderNameEntry.get_text();
    folder = &_management->AddFolder(*folder, new_name);
  }
  return *folder;
}

}  // namespace glight::gui
