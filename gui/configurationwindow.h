#ifndef CONFIGURATIONWINDOW_H
#define CONFIGURATIONWINDOW_H

#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/liststore.h>
#include <gtkmm/menu.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>
#include <gtkmm/window.h>

#include "../libtheatre/fixturetype.h"

/**
	@author Andre Offringa
*/
class ConfigurationWindow : public Gtk::Window {
	public:
		ConfigurationWindow(class Management &management);
		~ConfigurationWindow() { destroyPopupMenu(); }
		void Update() { fillFixturesList(); }
	private:
		void fillFixturesList();
		bool onAddButtonClicked(GdkEventButton* event);
		void onIncChannelButtonClicked();
		void onDecChannelButtonClicked();
		void onMenuItemClicked(enum FixtureType::FixtureClass cl);
		void updateFixture(const class Fixture *fixture);
		void destroyPopupMenu();
		static std::string getChannelString(const class Fixture& fixture);

		class Management &_management;

		Gtk::TreeView _fixturesListView;
		Glib::RefPtr<Gtk::ListStore> _fixturesListModel;
		struct FixturesListColumns : public Gtk::TreeModelColumnRecord
		{
			FixturesListColumns()
				{ add(_title); add(_type); add(_channels); add(_fixture); }
		
			Gtk::TreeModelColumn<Glib::ustring> _title;
			Gtk::TreeModelColumn<Glib::ustring> _type;
			Gtk::TreeModelColumn<Glib::ustring> _channels;
			Gtk::TreeModelColumn<class Fixture *> _fixture;
		} _fixturesListColumns;
		Gtk::ScrolledWindow _fixturesScrolledWindow;

		Gtk::VBox _mainBox;
		Gtk::HButtonBox _buttonBox;

		Gtk::Button _addButton, _incChannelButton, _decChannelButton;
		Gtk::Menu *_popupMenu;
		std::vector<Gtk::MenuItem *> _popupMenuItems;
};

#endif
