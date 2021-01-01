#include "addfixturewindow.h"

#include "eventtransmitter.h"

#include "../theatre/fixturecontrol.h"
#include "../theatre/fixturetype.h"
#include "../theatre/folder.h"
#include "../theatre/management.h"
#include "../theatre/theatre.h"

AddFixtureWindow::AddFixtureWindow(EventTransmitter *eventHub,
                                   Management &management)
    : _typeLabel("Type:"),
      _countLabel("Count:"),
      _countEntry(),
      _decCountButton("-"),
      _incCountButton("+"),
      _cancelButton("Cancel"),
      _addButton("Add"),
      _eventHub(*eventHub),
      _management(&management) {
  _grid.set_row_spacing(5);
  _grid.set_column_spacing(2);

  _grid.attach(_typeLabel, 0, 0, 1, 1);

  const std::vector<enum FixtureType::FixtureClass> &classes =
      FixtureType::GetClassList();
  for (enum FixtureType::FixtureClass fc : classes) {
    _typeCombo.append(FixtureType::ClassName(fc));
  }
  _typeCombo.set_active_text(FixtureType::ClassName(classes.front()));
  _grid.attach(_typeCombo, 1, 0, 3, 1);

  _grid.attach(_countLabel, 0, 1, 1, 1);
  _countEntry.set_text("1"), _grid.attach(_countEntry, 1, 1, 1, 1);
  _decCountButton.signal_clicked().connect([&]() { onDecCount(); });
  _grid.attach(_decCountButton, 2, 1, 1, 1);
  _incCountButton.signal_clicked().connect([&]() { onIncCount(); });
  _grid.attach(_incCountButton, 3, 1, 1, 1);

  _cancelButton.signal_clicked().connect([&]() { onCancel(); });
  _buttonBox.pack_start(_cancelButton);
  _addButton.signal_clicked().connect([&]() { onAdd(); });
  _buttonBox.pack_end(_addButton);
  _grid.attach(_buttonBox, 0, 2, 3, 1);

  add(_grid);
  show_all_children();
}

void AddFixtureWindow::onAdd() {
  std::string className = _typeCombo.get_active_text();
  enum FixtureType::FixtureClass fClass = FixtureType::NameToClass(className);
  int count = std::atoi(_countEntry.get_text().c_str());
  if (count > 0) {
    std::unique_lock<std::mutex> lock(_management->Mutex());

    FixtureType *type = dynamic_cast<FixtureType *>(
        _management->RootFolder().GetChildIfExists(className));
    if (!type) {
      // TODO we shouldn't use a type by its name, types should be editable etc
      type = &_management->Theatre().AddFixtureType(fClass);
      _management->RootFolder().Add(*type);
    }

    for (size_t fixIter = 0; fixIter != size_t(count); ++fixIter) {
      Position position = _management->Theatre().GetFreePosition();
      Fixture &fixture = _management->Theatre().AddFixture(*type);
      fixture.Position() = position;

      const std::vector<std::unique_ptr<FixtureFunction>> &functions =
          fixture.Functions();

      int number = 1;
      FixtureControl &control = _management->AddFixtureControl(
          fixture, _management->RootFolder() /* TODO */);
      for (size_t i = 0; i != functions.size(); ++i) {
        // std::stringstream funcName;
        // funcName << fixture.Name() << number;
        _management->AddSourceValue(control, i);
        ++number;
      }
    }

    lock.unlock();
    _eventHub.EmitUpdate();
    hide();
  }
}

void AddFixtureWindow::onDecCount() {
  int count = std::atoi(_countEntry.get_text().c_str());
  if (count > 1)
    count--;
  else
    count = 1;
  _countEntry.set_text(std::to_string(count));
}

void AddFixtureWindow::onIncCount() {
  int count = std::atoi(_countEntry.get_text().c_str());
  count++;
  _countEntry.set_text(std::to_string(count));
}
