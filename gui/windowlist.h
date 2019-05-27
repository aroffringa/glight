#ifndef WINDOW_LIST_H
#define WINDOW_LIST_H

#include <gtkmm/window.h>

#include <memory>
#include <vector>

class WindowList
{
public:
	void Add(std::unique_ptr<Gtk::Window> window)
	{
		window->signal_hide().connect([&]() { onHideWindow(window.get()); });
		_list.emplace_back(std::move(window));
	}
	
	const std::vector<std::unique_ptr<Gtk::Window>>& List() const { return _list; }
	
private:
	void onHideWindow(Gtk::Window* window)
	{
		auto iter = std::find_if(_list.begin(), _list.end(),
			[window](const std::unique_ptr<Gtk::Window>& elem) { return elem.get() == window; } );
		_list.erase(iter);
	}
	
	std::vector<std::unique_ptr<Gtk::Window>> _list;
};

#endif
