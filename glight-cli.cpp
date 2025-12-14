#include <iostream>
#include <string>

#include <ncurses.h>

#include "system/reader.h"
#include "system/settings.h"

#include "theatre/management.h"

#include "uistate/uistate.h"

namespace glight {

using uistate::FaderControlType;
using uistate::FaderSetState;
using uistate::FaderState;
using uistate::UIState;

namespace {

struct TextFader {
  std::string label;
  theatre::SourceValue* source;
  size_t line;
};

std::string Trim(const std::string& input, size_t width) {
  if (input.size() < width) return input;
  if (width > 5) {
    return input.substr(0, width - 2) + "..";
  } else {
    return input.substr(0, width);
  }
}

std::vector<TextFader> InitializeFaders(const UIState& state) {
  std::vector<TextFader> result;
  size_t n_lines = 0;
  for (const std::unique_ptr<FaderSetState>& fader_set : state.FaderSets()) {
    mvprintw(n_lines, 0, fader_set->name.c_str());  // TODO trim
    ++n_lines;

    for (const std::unique_ptr<FaderState>& fader : fader_set->faders) {
      if (fader->GetFaderType() == FaderControlType::Fader) {
        theatre::SourceValue* source = nullptr;
        if (!fader->GetSourceValues().empty())
          source = fader->GetSourceValues()[0];
        if (source) {
          TextFader& new_fader = result.emplace_back();
          new_fader.label = source->Name();
          new_fader.source = source;
          new_fader.line = n_lines;
          ++n_lines;
        }
      }
    }
  }
  return result;
}

void PrintState(const std::vector<TextFader>& faders, size_t width,
                size_t height) {
  for (const TextFader& fader : faders) {
    if (fader.line >= height) break;
    const theatre::ControlValue value = fader.source->A().Value();
    const std::string check_box = value ? "[X] " : "[ ] ";
    mvprintw(fader.line, 1, (check_box + Trim(fader.label, width - 5)).c_str());
  }
}

void RunCli(const std::string filename) {
  const glight::system::Settings settings = glight::system::LoadSettings();
  glight::theatre::Management management(settings);
  UIState state;
  glight::system::Read(filename, management, &state);
  management.GetUniverses().Open();
  management.Run();

  initscr();

  int height;
  int width;
  getmaxyx(stdscr, height, width);
  keypad(stdscr, TRUE);
  noecho();

  std::vector<TextFader> faders = InitializeFaders(state);
  PrintState(faders, width, height);

  size_t selected_fader = 0;
  bool running = true;
  do {
    move(faders[selected_fader].line, 2);
    const int c = getch();
    switch (c) {
      case KEY_UP:
        if (selected_fader > 0) --selected_fader;
        break;
      case KEY_DOWN:
        if (selected_fader + 1 < faders.size()) ++selected_fader;
        break;
      case ' ': {
        TextFader& fader = faders[selected_fader];
        theatre::ControlValue value = fader.source->A().Value();
        if (value) {
          value = theatre::ControlValue::Zero();
          printw(" ");
        } else {
          value = theatre::ControlValue::Max();
          printw("X");
        }
        fader.source->A().Set(value);
      } break;
      case 'q':
        running = false;
        break;
    }
  } while (running);

  endwin();

  management.BlackOut(false, 0.0f);
  // There is some time required for the black out to take effect.
  usleep(400000);
}

}  // namespace
}  // namespace glight

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    std::cout << "Syntax: glight-player <show-file>\n\n"
                 "glight-player can play a previously created gshow file "
                 "without requiring a graphical desktop.\n";
    return 0;
  }

  glight::RunCli(argv[1]);
}
