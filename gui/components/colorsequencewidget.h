#ifndef COLOR_SEQUENCE_WIDGET_H
#define COLOR_SEQUENCE_WIDGET_H

#include "colorselectwidget.h"

#include "../../theatre/color.h"

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
		_parent(parent),
		_minCount(1),
		_maxCount(0)
	{
		_allEqual.signal_clicked().connect(sigc::mem_fun(*this, &ColorSequenceWidget::onToggleEqual));
		pack_start(_allEqual, true, false);
		
		_widgets.emplace_back(new ColorSelectWidget(_parent));
		pack_start(*_widgets.back(), true, false);
		_widgets.back()->SignalColorChanged().connect(sigc::mem_fun(*this, &ColorSequenceWidget::onFirstColorChange));
		
		_minButton.set_sensitive(false);
		_minButton.signal_clicked().connect(sigc::mem_fun(*this, &ColorSequenceWidget::onDecreaseColors));
		_buttonBox.pack_start(_minButton);
		
		_plusButton.signal_clicked().connect(sigc::mem_fun(*this, &ColorSequenceWidget::onIncreaseColors));
		_buttonBox.pack_start(_plusButton);
		pack_end(_buttonBox, true, true);
		
		show_all_children();
	}
	
	void SetColors(const std::vector<Color>& colors)
	{
		if(_maxCount < colors.size())
			_maxCount = 0;
		if(colors.size() < _minCount)
			_minCount = colors.size();
		_allEqual.set_active(false);
		_widgets.clear();
		for(const Color& color : colors)
		{
			_widgets.emplace_back(new ColorSelectWidget(_parent));
			_widgets.back()->SetColor(color);
			pack_start(*_widgets.back(), true, false);
			_widgets.back()->show();
		}
		_minButton.set_sensitive(colors.size() > _minCount);
		_plusButton.set_sensitive(_maxCount==0 || _maxCount > colors.size());
	}
	
	std::vector<Color> GetColors() const
	{
		std::vector<Color> result;
		for(const std::unique_ptr<ColorSelectWidget>& w : _widgets)
			result.emplace_back(w->GetColor());
		return result;
	}
	
	void SetMinCount(size_t minCount)
	{
		if(_maxCount < minCount)
			SetMaxCount(minCount);
		while(_widgets.size() < minCount)
			onIncreaseColors();
		_minCount = minCount;
		_minButton.set_sensitive(_widgets.size() > _minCount);
		_plusButton.set_sensitive(_maxCount == 0 || _widgets.size() < _maxCount);
	}
	
	void SetMaxCount(size_t maxCount)
	{
		if(_minCount > maxCount)
			SetMinCount(maxCount);
		_maxCount = maxCount;
		if(_maxCount!=0)
		{
			if(_widgets.size() > _maxCount)
				_widgets.resize(_maxCount);
			_minButton.set_sensitive(_widgets.size() > _minCount);
		}
		_plusButton.set_sensitive(_maxCount == 0 || _widgets.size() < _maxCount);
	}
	
private:
	Gtk::CheckButton _allEqual;
	std::vector<std::unique_ptr<class ColorSelectWidget>> _widgets;
	Gtk::ButtonBox _buttonBox;
	Gtk::Button _plusButton, _minButton;
	Gtk::Window* _parent;
	size_t _minCount, _maxCount;
	
	void onDecreaseColors()
	{
		if(_widgets.size() > _minCount)
		{
			_widgets.pop_back();
			_minButton.set_sensitive(_widgets.size() > _minCount);
			_plusButton.set_sensitive(_maxCount == 0 || _widgets.size() < _maxCount);
		}
	}
	void onIncreaseColors()
	{
		if(_maxCount==0 || _widgets.size() < _maxCount)
		{
			_widgets.emplace_back(new ColorSelectWidget(_parent));
			if(_allEqual.get_active())
			{
				_widgets.back()->SetColor(_widgets.front()->GetColor());
				_widgets.back()->set_sensitive(false);
			}
			_minButton.set_sensitive(_widgets.size() > _minCount);
			_plusButton.set_sensitive(_maxCount == 0 || _widgets.size() < _maxCount);
			pack_start(*_widgets.back(), true, false);
			_widgets.back()->show();
			queue_resize();
		}
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
