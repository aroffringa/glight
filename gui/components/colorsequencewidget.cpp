#include "colorsequencewidget.h"

#include "../windows/gradientwindow.h"

#include <glibmm/main.h>
#include <gtkmm/main.h>

void ColorSequenceWidget::onGradient() {
  if (!_allEqual.get_active() && _widgets.size() > 2) {
    GradientWindow window(_widgets.size());
    window.set_modal(true);
    Gtk::Main::run(window);

    if (window.Result()) {
      std::vector<Color> colors = window.GetColors();
      _widgets.front()->SetColor(colors.front());
      _widgets.back()->SetColor(colors.back());

      for (size_t i = 1; i < _widgets.size() - 1; ++i) {
        double floatIndex =
            double(i) * (colors.size() - 1) / (_widgets.size() - 1);
        Color leftColor = colors[floor(floatIndex)];
        Color rightColor = colors[floor(floatIndex) + 1];
        double balance = floatIndex - floor(floatIndex);
        unsigned red = (rightColor.Red() * balance +
                        leftColor.Red() * (1.0 - balance)),
                 green = (rightColor.Green() * balance +
                          leftColor.Green() * (1.0 - balance)),
                 blue = (rightColor.Blue() * balance +
                         leftColor.Blue() * (1.0 - balance));
        _widgets[i]->SetColor(Color(red, green, blue));
      }
    }
  }
}
