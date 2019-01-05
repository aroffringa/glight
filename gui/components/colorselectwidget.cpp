#include "colorselectwidget.h"

#include <gtkmm/colorchooserdialog.h>

ColorSelectWidget::ColorSelectWidget(Gtk::Window* parent) :
	_parent(parent),
	_label("Color:"),
	_button("Change..."),
	_colorR(1.0), _colorG(1.0), _colorB(1.0)
{
	pack_start(_label);
	_button.signal_clicked().connect(sigc::mem_fun(*this, &ColorSelectWidget::onColorClicked));
	pack_end(_button);
	_area.signal_draw().connect(sigc::mem_fun(*this, &ColorSelectWidget::onColorAreaDraw));
	pack_end(_area, true, true, 5);
	show_all_children();
}

void ColorSelectWidget::onColorClicked()
{
	Gdk::RGBA color;
	color.set_red(_colorR);
	color.set_green(_colorG);
	color.set_blue(_colorB);
  color.set_alpha(1.0); //opaque
	
	Gtk::ColorChooserDialog dialog("Select color for fixture");
	dialog.set_transient_for(*_parent);
	dialog.set_rgba(color);
	dialog.set_use_alpha(false);
	const int result = dialog.run();

	//Handle the response:
	if(result == Gtk::RESPONSE_OK)
	{
		//Store the chosen color:
		color = dialog.get_rgba();
		_colorR = color.get_red();
		_colorG = color.get_green();
		_colorB = color.get_blue();
		_signalColorChanged();
		_area.queue_draw();
	}
}

bool ColorSelectWidget::onColorAreaDraw(const Cairo::RefPtr<Cairo::Context>& cr)
{
	cr->set_source_rgb(_colorR, _colorG, _colorB);
  cr->paint();
  return true;
}

