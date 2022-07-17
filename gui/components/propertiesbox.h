#ifndef GUI_PROPERTIES_BOX_H_
#define GUI_PROPERTIES_BOX_H_

#include "../../theatre/properties/propertyset.h"

#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/entry.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/widget.h>

#include <memory>
#include <vector>

namespace glight::gui {

class PropertiesBox : public Gtk::VBox {
 public:
  PropertiesBox();

  void SetPropertySet(theatre::PropertySet *propertySet) {
    _propertySet = propertySet;
    fillProperties();
  }

  void Clear();

 private:
  struct PropertyRow {
    theatre::Property *_property;
    std::vector<std::unique_ptr<Gtk::Widget>> _widgets;
  };

  theatre::PropertySet *_propertySet;
  std::vector<PropertyRow> _rows;
  Gtk::Label _typeLabel;
  Gtk::Grid _grid;
  Gtk::ButtonBox _propertiesButtonBox;
  Gtk::Button _applyButton;

  void onApplyClicked();
  void fillProperties();
};

}  // namespace glight::gui

#endif
