#ifndef COLOR_SEQUENCE_WIDGET_H
#define COLOR_SEQUENCE_WIDGET_H

#include "colorselectwidget.h"

#include "../../libtheatre/color.h"

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/checkbutton.h>

#include <vector>

class ColorSequenceWidget : public Gtk::VBox
{
public:
	ColorSequenceWidget(Gtk::Window* parent) :
		_allEqual("Use one color for all"),
		_plusButton("+"),
		_minButton("-"),
		_parent(parent)
	{
		_allEqual.signal_clicked().connect(sigc::mem_fun(*this, &ColorSequenceWidget::onToggleEqual));
		pack_start(_allEqual, true, false);
		
		_widgets.emplace_back(new ColorSelectWidget(_parent));
		pack_start(*_widgets.back(), true, false);
		_widgets.back()->SignalColorChanged().connect(sigc::mem_fun(*this, &ColorSequenceWidget::onFirstColorChange));
		
		_minButton.signal_clicked().connect(sigc::mem_fun(*this, &ColorSequenceWidget::onDecreaseColors));
		_buttonBox.pack_start(_minButton);
		_plusButton.signal_clicked().connect(sigc::mem_fun(*this, &ColorSequenceWidget::onIncreaseColors));
		_buttonBox.pack_start(_plusButton);
		pack_end(_buttonBox, true, true);
		
		show_all_children();
	}
	
	void SetColors(const std::vector<Color>& colors)
	{
		_allEqual.set_active(false);
		_widgets.clear();
		for(const Color& color : colors)
		{
			_widgets.emplace_back(new ColorSelectWidget(_parent));
			_widgets.back()->SetColor(color);
			pack_start(*_widgets.back(), true, false);
			_widgets.back()->show();
		}
	}
	
	std::vector<Color> GetColors() const
	{
		std::vector<Color> result;
		for(const std::unique_ptr<ColorSelectWidget>& w : _widgets)
			result.emplace_back(w->GetColor());
		return result;
	}
	
private:
	Gtk::CheckButton _allEqual;
	std::vector<std::unique_ptr<class ColorSelectWidget>> _widgets;
	Gtk::ButtonBox _buttonBox;
	Gtk::Button _plusButton, _minButton;
	Gtk::Window* _parent;
	
	void onDecreaseColors()
	{
		if(_widgets.size() > 1)
			_widgets.pop_back();
	}
	void onIncreaseColors()
	{
		_widgets.emplace_back(new ColorSelectWidget(_parent));
		if(_allEqual.get_active())
		{
			_widgets.back()->SetColor(_widgets.front()->GetColor());
			_widgets.back()->set_sensitive(false);
		}
		pack_start(*_widgets.back(), true, false);
		_widgets.back()->show();
		this->queue_resize();
	}
	void onFirstColorChange()
	{
		if(_allEqual.get_active())
		{
			Color c = _widgets.front()->GetColor();
			for(size_t i=1; i!=_widgets.size(); ++i)
			{
				_widgets[i]->SetColor(c);
				_widgets[i]->set_sensitive(false);
			}
		}
	}
	void onToggleEqual()
	{
		if(_allEqual.get_active())
		{
			Color c = _widgets.front()->GetColor();
			for(size_t i=1; i!=_widgets.size(); ++i)
			{
				_widgets[i]->SetColor(c);
				_widgets[i]->set_sensitive(false);
			}
		}
		else {
			for(size_t i=1; i!=_widgets.size(); ++i)
			{
				_widgets[i]->set_sensitive(true);
			}
		}
	}
};

#endif
