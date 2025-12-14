#include "functions.h"

#include <functional>
#include <optional>

#include <gtkmm/colorchooserdialog.h>

#include "instance.h"

#include "uistate/faderstate.h"
#include "uistate/uistate.h"

#include "theatre/controllable.h"
#include "theatre/management.h"

namespace glight::gui {

void OpenColorDialog(std::unique_ptr<Gtk::Dialog> &dialog, Gtk::Window &parent,
                     const theatre::Color &start_color,
                     std::function<void(theatre::Color)> callback) {
  dialog =
      std::make_unique<Gtk::ColorChooserDialog>("Select color for fixture");
  Gtk::ColorChooserDialog &color_dialog =
      static_cast<Gtk::ColorChooserDialog &>(*dialog);
  color_dialog.set_transient_for(parent);
  Gdk::RGBA rgba_color;
  rgba_color.set_red(start_color.RedRatio());
  rgba_color.set_green(start_color.GreenRatio());
  rgba_color.set_blue(start_color.BlueRatio());
  rgba_color.set_alpha(1.0);  // opaque
  color_dialog.set_rgba(rgba_color);
  color_dialog.set_use_alpha(false);
  const std::vector<theatre::Color> color_set = theatre::Color::DefaultSet40();
  std::vector<Gdk::RGBA> colors;
  colors.reserve(color_set.size());
  for (const theatre::Color &color : color_set) {
    colors.emplace_back().set_rgba(color.RedRatio(), color.GreenRatio(),
                                   color.BlueRatio());
  }
  color_dialog.add_palette(Gtk::Orientation::HORIZONTAL, 10, colors);
  dialog->signal_response().connect([callback, &dialog](int response) {
    if (response == Gtk::ResponseType::OK) {
      Gtk::ColorChooserDialog &color_dialog =
          static_cast<Gtk::ColorChooserDialog &>(*dialog);
      const Gdk::RGBA rgba_color = color_dialog.get_rgba();
      const theatre::Color color = theatre::Color::FromRatio(
          rgba_color.get_red(), rgba_color.get_green(), rgba_color.get_blue());
      callback(color);
    }
    dialog.reset();
  });
  dialog->show();
}

void AssignFader(theatre::Controllable &controllable) {
  if (controllable.NInputs() == 1) {
    theatre::Management &management = Instance::Management();
    glight::uistate::FaderState *fader =
        Instance::State().GetFirstUnassignedFader();
    if (fader) {
      theatre::SourceValue *source_value =
          management.GetSourceValue(controllable, 0);
      fader->SetSourceValues({source_value});
      fader->SignalChange();
    }
  }
}

}  // namespace glight::gui
