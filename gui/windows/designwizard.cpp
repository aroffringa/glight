#include "designwizard.h"

#include <memory>
#include <ranges>

#include "gui/eventtransmitter.h"
#include "gui/fixtureselection.h"
#include "gui/functions.h"
#include "gui/instance.h"

#include "gui/components/colorselectwidget.h"

#include "gui/state/guistate.h"

#include "theatre/chase.h"
#include "theatre/colordeduction.h"
#include "theatre/effect.h"
#include "theatre/fixture.h"
#include "theatre/fixturecontrol.h"
#include "theatre/folder.h"
#include "theatre/folderoperations.h"
#include "theatre/management.h"
#include "theatre/theatre.h"
#include "theatre/timesequence.h"

#include "theatre/design/colorpreset.h"
#include "theatre/design/designinfo.h"
#include "theatre/design/rotation.h"

namespace glight::gui {

using system::ObservingPtr;
using theatre::AutoDesign;

DesignWizard::DesignWizard()
    : _selectLabel("Select fixtures:"),
      _fixtureList(),
      _objectBrowser(),

      _reorderWidget(),

      _typeFrameP3("Type"),
      _deductionFrameP3("Colour deduction"),
      _colorPresetBtn("Colour preset"),
      _runningLightBtn("Running light"),
      _singleColorBtn("Color variation"),
      _shiftColorsBtn("Shifting colours"),
      _increaseBtn("Increasing colours"),
      _rotationBtn("Rotating colours"),
      _vuMeterBtn("VU meter"),
      _fireBtn("Fire"),
      _deduceWhite("White from RGB"),
      _deduceAmber("Amber from RGB"),
      _deduceUV("UV from RGB"),
      _deduceLime("Lime from RGB"),

      _colorsWidgetP4(this),
      _parentFolderCombo(),
      _newFolderCB("New folder"),
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
      _rotForwardRB("Forward direction"),
      _rotBackwardRB("Backward direction"),
      _rotForwardReturnRB("Forward and return"),
      _nextButton("Next"),
      _currentPage(Page1_SelFixtures) {
  initPage1();
  initPage2();

  _mainBox.append(_vBoxPage1);
  _colorsWidgetP4.set_expand(true);
  _fixtureList.set_expand(true);
  _reorderWidget.set_expand(true);
  _typeFrameP3.set_expand(true);
  _deductionFrameP3.set_expand(true);

  _buttonBox.set_homogeneous(true);

  _nextButton.signal_clicked().connect(
      sigc::mem_fun(*this, &DesignWizard::onNextClicked));
  _buttonBox.append(_nextButton);
  _mainBox.append(_buttonBox);

  set_child(_mainBox);
  _mainBox.show();

  _fixtureList.Select(Instance::Selection().Selection());
}

DesignWizard::~DesignWizard() = default;

void DesignWizard::initPage1() {
  _vBoxPage1.append(_notebook);

  _notebook.append_page(_vBoxPage1a, "Fixtures");
  _vBoxPage1a.append(_selectLabel);

  _vBoxPage1a.append(_fixtureList);

  _notebook.append_page(_vBoxPage1b, "Any controllables");

  _objectBrowser.SignalSelectionChange().connect(
      [&]() { onControllableSelected(); });
  _objectBrowser.SignalObjectActivated().connect(
      [&](const ObservingPtr<theatre::FolderObject> &object) {
        addControllable(object);
      });
  _vBoxPage1b.append(_objectBrowser);

  _addControllableButton.set_image_from_icon_name("go-down");
  _addControllableButton.set_sensitive(false);
  _addControllableButton.signal_clicked().connect(
      sigc::mem_fun(*this, &DesignWizard::onAddControllable));
  _controllableButtonBox.append(_addControllableButton);

  _removeControllableButton.set_image_from_icon_name("go-up");
  _removeControllableButton.signal_clicked().connect(
      sigc::mem_fun(*this, &DesignWizard::onRemoveControllable));
  _controllableButtonBox.append(_removeControllableButton);

  _controllableButtonBox.set_halign(Gtk::Align::CENTER);
  _vBoxPage1b.append(_controllableButtonBox);

  _controllablesListModel = Gtk::ListStore::create(_controllablesListColumns);
  _controllablesListView.set_model(_controllablesListModel);
  _controllablesListView.append_column("Controllable",
                                       _controllablesListColumns._title);
  _controllablesListView.append_column("Path", _controllablesListColumns._path);
  _controllablesListView.set_rubber_banding(true);
  _controllablesListView.get_selection()->set_mode(
      Gtk::SelectionMode::MULTIPLE);
  _controllablesScrolledWindow.set_child(_controllablesListView);
  _vBoxPage1b.append(_controllablesScrolledWindow);
}

void DesignWizard::initPage2() {
  _vBoxPage3Type.append(_colorPresetBtn);
  _runningLightBtn.set_group(_colorPresetBtn);
  _vBoxPage3Type.append(_runningLightBtn);
  _singleColorBtn.set_group(_colorPresetBtn);
  _vBoxPage3Type.append(_singleColorBtn);
  _shiftColorsBtn.set_group(_colorPresetBtn);
  _vBoxPage3Type.append(_shiftColorsBtn);
  _increaseBtn.set_group(_colorPresetBtn);
  _vBoxPage3Type.append(_increaseBtn);
  _rotationBtn.set_group(_colorPresetBtn);
  _vBoxPage3Type.append(_rotationBtn);
  _vuMeterBtn.set_group(_colorPresetBtn);
  _vBoxPage3Type.append(_vuMeterBtn);
  _fireBtn.set_group(_colorPresetBtn);
  _vBoxPage3Type.append(_fireBtn);

  _typeFrameP3.set_child(_vBoxPage3Type);
  _vBoxPage3.append(_typeFrameP3);

  _deduceWhite.set_active(true);
  _vBoxPage3Deduction.append(_deduceWhite);
  _deduceAmber.set_active(true);
  _vBoxPage3Deduction.append(_deduceAmber);
  _deduceUV.set_active(true);
  _vBoxPage3Deduction.append(_deduceUV);
  _deduceLime.set_active(true);
  _vBoxPage3Deduction.append(_deduceLime);

  _deductionFrameP3.set_child(_vBoxPage3Deduction);
  _vBoxPage3.append(_deductionFrameP3);
}

void DesignWizard::initPage4_1RunningLight() {
  initPage4Destination("Running light");
  _vBoxPage4.append(_colorsWidgetP4);
  _vBoxPage4.append(_increasingRunRB);
  _decreasingRunRB.set_group(_increasingRunRB);
  _vBoxPage4.append(_decreasingRunRB);
  _backAndForthRunRB.set_group(_increasingRunRB);
  _vBoxPage4.append(_backAndForthRunRB);
  _inwardRunRB.set_group(_increasingRunRB);
  _vBoxPage4.append(_inwardRunRB);
  _outwardRunRB.set_group(_increasingRunRB);
  _vBoxPage4.append(_outwardRunRB);
  _randomRunRB.set_group(_increasingRunRB);
  _vBoxPage4.append(_randomRunRB);
}

void DesignWizard::initPage4_2SingleColor() {
  initPage4Destination("Same color run");
  _vBoxPage4.append(_colorsWidgetP4);
  _vBoxPage4.append(_variationLabel);
  _variation.set_range(0, 100);
  _variation.set_increments(1.0, 10.0);
  _vBoxPage4.append(_variation);
}

void DesignWizard::initPage4_3ShiftColors() {
  initPage4Destination("Shift colors");
  _vBoxPage4.append(_colorsWidgetP4);
  _vBoxPage4.append(_shiftIncreasingRB);
  _shiftDecreasingRB.set_group(_shiftIncreasingRB);
  _vBoxPage4.append(_shiftDecreasingRB);
  _shiftBackAndForthRB.set_group(_shiftIncreasingRB);
  _vBoxPage4.append(_shiftBackAndForthRB);
  _shiftRandomRB.set_group(_shiftIncreasingRB);
  _vBoxPage4.append(_shiftRandomRB);
}

void DesignWizard::initPage4_4VUMeter() {
  initPage4Destination("VUMeter");
  _vBoxPage4.append(_colorsWidgetP4);
  _vBoxPage4.append(_vuIncreasingRB);
  _vuDecreasingRB.set_group(_vuIncreasingRB);
  _vBoxPage4.append(_vuDecreasingRB);
  _vuInwardRunRB.set_group(_vuIncreasingRB);
  _vBoxPage4.append(_vuInwardRunRB);
  _vuOutwardRunRB.set_group(_vuIncreasingRB);
  _vBoxPage4.append(_vuOutwardRunRB);
}

void DesignWizard::initPage4_5ColorPreset() {
  initPage4Destination("Color preset");
  _vBoxPage4.append(_colorsWidgetP4);
  _vBoxPage4.append(_eachFixtureSeparatelyCB);
}

void DesignWizard::initPage4_6Increasing() {
  initPage4Destination("Increase chase");
  _vBoxPage4.append(_colorsWidgetP4);
  _vBoxPage4.append(_incForwardRB);
  _incBackwardRB.set_group(_incForwardRB);
  _vBoxPage4.append(_incBackwardRB);
  _incForwardReturnRB.set_group(_incForwardRB);
  _vBoxPage4.append(_incForwardReturnRB);
  _incBackwardReturnRB.set_group(_incForwardRB);
  _vBoxPage4.append(_incBackwardReturnRB);
}

void DesignWizard::initPage4_7Rotation() {
  initPage4Destination("Rotation");
  _vBoxPage4.append(_colorsWidgetP4);
  _vBoxPage4.append(_rotForwardRB);
  _rotBackwardRB.set_group(_rotForwardRB);
  _vBoxPage4.append(_rotBackwardRB);
  _rotForwardReturnRB.set_group(_rotForwardRB);
  _vBoxPage4.append(_rotForwardReturnRB);
}

void DesignWizard::initPage4_8Fire() {
  initPage4Destination("Fire");
  _vBoxPage4.append(_colorsWidgetP4);
}

void DesignWizard::initPage4Destination(const std::string &name) {
  _parentLabel.set_text("Destination:");
  _vBoxPage4.append(_parentLabel);
  theatre::Folder &folder = getCurrentFolder();
  _parentFolderCombo.Select(folder);
  _vBoxPage4.append(_parentFolderCombo);
  _newFolderCB.set_active(true);
  const std::string new_name = folder.GetAvailableName(name);
  _nameEntry.set_text(new_name);
  _folderNameBox.append(_nameEntry);
  _folderNameBox.append(_newFolderCB);
  _vBoxPage4.append(_folderNameBox);
}

theatre::Folder &DesignWizard::getCurrentFolder() const {
  theatre::Folder *folder = dynamic_cast<theatre::Folder *>(
      Instance::Management().GetObjectFromPathIfExists(_currentPath));
  if (folder)
    return *folder;
  else
    return Instance::Management().RootFolder();
}

theatre::DesignInfo DesignWizard::GetDesign() const {
  return theatre::DesignInfo{&Instance::Management(), &makeDestinationFolder(),
                             _nameEntry.get_text(), &_selectedControllables,
                             colorDeduction()};
}

void DesignWizard::onNextClicked() {
  theatre::Management &management = Instance::Management();
  gui::EventTransmitter &events = Instance::Events();
  _mainBox.remove(_buttonBox);
  switch (_currentPage) {
    case Page1_SelFixtures: {
      _selectedControllables.clear();
      if (_notebook.get_current_page() == 0) {
        for (std::vector<system::ObservingPtr<theatre::Fixture>> fixtures =
                 _fixtureList.Selection();
             const system::ObservingPtr<theatre::Fixture> &fixture : fixtures) {
          _selectedControllables.emplace_back(
              management.GetFixtureControl(*fixture));
        }
      } else {
        for (const auto &iter : _controllablesListModel->children()) {
          _selectedControllables.emplace_back(
              (iter)[_controllablesListColumns._controllable]);
        }
      }
      _mainBox.remove(_vBoxPage1);

      _reorderWidget.SetList(
          std::vector<system::ObservingPtr<theatre::NamedObject>>(
              _selectedControllables.begin(), _selectedControllables.end()));
      _mainBox.append(_reorderWidget);
      _reorderWidget.show();
      _currentPage = Page2_Order;
    } break;

    case Page2_Order: {
      std::vector<ObservingPtr<theatre::NamedObject>> list =
          _reorderWidget.GetList();
      _selectedControllables.clear();
      for (const ObservingPtr<theatre::NamedObject> &object : list)
        _selectedControllables.emplace_back(
            static_cast<ObservingPtr<theatre::Controllable>>(object));
      _mainBox.remove(_reorderWidget);
      _mainBox.append(_vBoxPage3);
      _currentPage = Page3_SelType;
    } break;

    case Page3_SelType:
      _mainBox.remove(_vBoxPage3);
      if (_colorPresetBtn.get_active()) {
        initPage4_5ColorPreset();
        _colorsWidgetP4.SetColors(
            std::vector<Color>(_selectedControllables.size(), Color::White()));
        _colorsWidgetP4.SetMinCount(_selectedControllables.size());
        _colorsWidgetP4.SetMaxCount(_selectedControllables.size());
        _mainBox.append(_vBoxPage4);
        _currentPage = Page4_5_ColorPreset;
      } else if (_runningLightBtn.get_active()) {
        initPage4_1RunningLight();
        _colorsWidgetP4.SetColors(
            std::vector<Color>(_selectedControllables.size(), Color::White()));
        _colorsWidgetP4.SetMaxCount(_selectedControllables.size());
        _mainBox.append(_vBoxPage4);
        _currentPage = Page4_1_RunningLight;
      } else if (_singleColorBtn.get_active()) {
        initPage4_2SingleColor();
        _mainBox.append(_vBoxPage4);
        _currentPage = Page4_2_SingleColor;
      } else if (_shiftColorsBtn.get_active()) {
        initPage4_3ShiftColors();
        _colorsWidgetP4.SetMinCount(1);
        _colorsWidgetP4.SetMaxCount(_selectedControllables.size());
        _colorsWidgetP4.SetColors(
            std::vector<Color>(_selectedControllables.size(), Color::White()));
        _mainBox.append(_vBoxPage4);
        _currentPage = Page4_3_ShiftingColors;
      } else if (_vuMeterBtn.get_active()) {
        initPage4_4VUMeter();
        _colorsWidgetP4.SetMinCount(_selectedControllables.size());
        _colorsWidgetP4.SetMaxCount(_selectedControllables.size());
        _mainBox.append(_vBoxPage4);
        _currentPage = Page4_4_VUMeter;
      } else if (_increaseBtn.get_active()) {
        initPage4_6Increasing();
        _colorsWidgetP4.SetMinCount(_selectedControllables.size());
        _colorsWidgetP4.SetMaxCount(_selectedControllables.size());
        _mainBox.append(_vBoxPage4);
        _currentPage = Page4_6_Increasing;
      } else if (_rotationBtn.get_active()) {
        initPage4_7Rotation();
        _colorsWidgetP4.SetMinCount(_selectedControllables.size());
        _colorsWidgetP4.SetMaxCount(0);
        _mainBox.append(_vBoxPage4);
        _currentPage = Page4_7_Rotation;
      } else {
        initPage4_8Fire();
        _colorsWidgetP4.SetMinCount(1);
        _colorsWidgetP4.SetMaxCount(0);
        _mainBox.append(_vBoxPage4);
        _currentPage = Page4_8_Fire;
      }
      break;

    case Page4_1_RunningLight: {
      using theatre::RunType;
      RunType runType;
      if (_increasingRunRB.get_active())
        runType = RunType::IncreasingRun;
      else if (_decreasingRunRB.get_active())
        runType = RunType::DecreasingRun;
      else if (_backAndForthRunRB.get_active())
        runType = RunType::BackAndForthRun;
      else if (_inwardRunRB.get_active())
        runType = RunType::InwardRun;
      else if (_outwardRunRB.get_active())
        runType = RunType::OutwardRun;
      else  // if(_randomRunRB.get_active())
        runType = RunType::RandomRun;
      std::unique_lock lock(management.Mutex());
      theatre::Chase &chase = AutoDesign::MakeRunningLight(
          GetDesign(), _colorsWidgetP4.GetSelection(), runType);
      lock.unlock();
      events.EmitUpdate();
      AssignFader(chase);
      hide();
    } break;

    case Page4_2_SingleColor: {
      std::unique_lock lock(management.Mutex());
      theatre::Chase &chase = AutoDesign::MakeColorVariation(
          GetDesign(), _colorsWidgetP4.GetSelection(), _variation.get_value());
      lock.unlock();
      events.EmitUpdate();
      AssignFader(chase);
      hide();
    } break;

    case Page4_3_ShiftingColors: {
      using theatre::ShiftType;
      ShiftType shiftType;
      if (_shiftIncreasingRB.get_active())
        shiftType = ShiftType::IncreasingShift;
      else if (_shiftDecreasingRB.get_active())
        shiftType = ShiftType::DecreasingShift;
      else if (_shiftBackAndForthRB.get_active())
        shiftType = ShiftType::BackAndForthShift;
      else
        shiftType = ShiftType::RandomShift;
      std::unique_lock lock(management.Mutex());
      theatre::Chase &chase = AutoDesign::MakeColorShift(
          GetDesign(), _colorsWidgetP4.GetSelection(), shiftType);
      lock.unlock();
      events.EmitUpdate();
      AssignFader(chase);
      hide();
    } break;

    case Page4_4_VUMeter: {
      using theatre::VUMeterDirection;
      VUMeterDirection direction;
      if (_vuIncreasingRB.get_active())
        direction = VUMeterDirection::VUIncreasing;
      else if (_vuDecreasingRB.get_active())
        direction = VUMeterDirection::VUDecreasing;
      else if (_vuInwardRunRB.get_active())
        direction = VUMeterDirection::VUInward;
      else  // if(_vuOutwardRunRB.get_active())
        direction = VUMeterDirection::VUOutward;
      std::unique_lock lock(management.Mutex());
      glight::theatre::Controllable &vu_meter = AutoDesign::MakeVUMeter(
          GetDesign(), _colorsWidgetP4.GetSelection(), direction);
      lock.unlock();
      events.EmitUpdate();
      AssignFader(vu_meter);
      hide();
    } break;

    case Page4_5_ColorPreset: {
      std::unique_lock lock(management.Mutex());
      if (_eachFixtureSeparatelyCB.get_active()) {
        MakeColorPresetPerFixture(GetDesign(), _colorsWidgetP4.GetSelection());
        lock.unlock();
        events.EmitUpdate();
      } else {
        glight::theatre::PresetCollection &preset =
            MakeColorPreset(GetDesign(), _colorsWidgetP4.GetSelection());
        lock.unlock();
        events.EmitUpdate();
        AssignFader(preset);
      }
      hide();
    } break;

    case Page4_6_Increasing: {
      using theatre::IncreasingType;
      IncreasingType incType;
      if (_incForwardRB.get_active())
        incType = IncreasingType::IncForward;
      else if (_incBackwardRB.get_active())
        incType = IncreasingType::IncBackward;
      else if (_incForwardReturnRB.get_active())
        incType = IncreasingType::IncForwardReturn;
      else  // if(_incBackwardReturnRB.get_active())
        incType = IncreasingType::IncBackwardReturn;
      std::unique_lock lock(management.Mutex());
      glight::theatre::Chase &chase = AutoDesign::MakeIncreasingChase(
          GetDesign(), _colorsWidgetP4.GetSelection(), incType);
      lock.unlock();
      events.EmitUpdate();
      AssignFader(chase);
      hide();
    } break;

    case Page4_7_Rotation: {
      using theatre::RotationType;
      RotationType type;
      if (_rotForwardRB.get_active())
        type = RotationType::Forward;
      else if (_rotBackwardRB.get_active())
        type = RotationType::Backward;
      else  // if (_rotForwardReturnRB.get_active())
        type = RotationType::ForwardBackward;
      std::unique_lock lock(management.Mutex());
      glight::theatre::TimeSequence &rotation =
          MakeRotation(GetDesign(), _colorsWidgetP4.GetSelection(), type);
      lock.unlock();
      events.EmitUpdate();
      AssignFader(rotation);
      hide();
    } break;

    case Page4_8_Fire: {
      using theatre::RotationType;
      std::unique_lock lock(management.Mutex());
      theatre::Effect &fire =
          AutoDesign::MakeFire(GetDesign(), _colorsWidgetP4.GetSelection());
      lock.unlock();
      events.EmitUpdate();
      AssignFader(fire);
      hide();
    } break;
  }
  _mainBox.append(_buttonBox);
}

void DesignWizard::addControllable(
    const system::ObservingPtr<theatre::FolderObject> &object) {
  theatre::Controllable *controllable =
      dynamic_cast<theatre::Controllable *>(object.Get());
  if (controllable) {
    Gtk::TreeModel::iterator iter = _controllablesListModel->append();
    Gtk::TreeModel::Row &row = *iter;
    if (iter) {
      row[_controllablesListColumns._controllable] =
          static_cast<ObservingPtr<theatre::Controllable>>(object);
      row[_controllablesListColumns._title] = controllable->Name();
      row[_controllablesListColumns._path] = controllable->Parent().FullPath();
    }
  }
}

void DesignWizard::onAddControllable() {
  ObservingPtr<theatre::FolderObject> object = _objectBrowser.SelectedObject();
  if (object) addControllable(std::move(object));
}

void DesignWizard::onRemoveControllable() {
  std::vector<Gtk::TreeModel::Path> rows =
      _controllablesListView.get_selection()->get_selected_rows();

  for (Gtk::TreeModel::Path &elementIter : std::ranges::reverse_view(rows))
    _controllablesListModel->erase(
        _controllablesListModel->get_iter(elementIter));
}

void DesignWizard::onControllableSelected() {
  theatre::Controllable *object = dynamic_cast<theatre::Controllable *>(
      _objectBrowser.SelectedObject().Get());
  _addControllableButton.set_sensitive(object != nullptr);
}

theatre::ColorDeduction DesignWizard::colorDeduction() const {
  theatre::ColorDeduction deduction;
  deduction.whiteFromRGB = _deduceWhite.get_active();
  deduction.amberFromRGB = _deduceAmber.get_active();
  deduction.uvFromRGB = _deduceUV.get_active();
  deduction.limeFromRGB = _deduceLime.get_active();
  return deduction;
}

theatre::Folder &DesignWizard::makeDestinationFolder() const {
  theatre::Folder *folder = &_parentFolderCombo.Selection();
  if (_newFolderCB.get_active()) {
    const std::string new_name = _nameEntry.get_text();
    theatre::Management &management = Instance::Management();
    folder = &management.AddFolder(*folder, new_name);
  }
  return *folder;
}

}  // namespace glight::gui
