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

#include "../theatre/fixturetype.h"

#include "recursionlock.h"

#include <memory>

/**
	@author Andre Offringa
*/
class FixtureListWindow : public Gtk::Window {
	public:
		FixtureListWindow(class EventTransmitter* eventHub, class Management& management,
			class FixtureSelection* globalSelection);
		~FixtureListWindow();
		
	private:
		void onChangeManagement(class Management& management)
		{
			_management = &management;
			fillFixturesList();
		}
		void update() { fillFixturesList(); }
		void fillFixturesList();
		void onNewButtonClicked();
		void onRemoveButtonClicked();
		void onIncChannelButtonClicked();
		void onDecChannelButtonClicked();
		void onSetChannelButtonClicked();
		void onMenuItemClicked(enum FixtureType::FixtureClass cl);
		void updateFixture(const class Fixture *fixture);
		static std::string getChannelString(const class Fixture& fixture);
		void onSelectionChanged();
		void onGlobalSelectionChange();

		class EventTransmitter* _eventHub;
		class Management* _management;
		class FixtureSelection* _globalSelection;
		
		sigc::connection
			_changeManagementConnection,
			_updateControllablesConnection,
			_globalSelectionConnection;
		RecursionLock _recursionLock;

		Gtk::TreeView _fixturesListView;
		Glib::RefPtr<Gtk::ListStore> _fixturesListModel;
		struct FixturesListColumns : public Gtk::TreeModelColumnRecord
		{
			FixturesListColumns()
				{ add(_title); add(_type); add(_channels); add(_symbol); add(_fixture); }
		
			Gtk::TreeModelColumn<Glib::ustring> _title, _type, _channels, _symbol;
			Gtk::TreeModelColumn<class Fixture *> _fixture;
		} _fixturesListColumns;
		Gtk::ScrolledWindow _fixturesScrolledWindow;

		Gtk::VBox _mainBox;
		Gtk::HButtonBox _buttonBox;

		Gtk::Button _newButton, _removeButton;
		Gtk::Button _incChannelButton, _decChannelButton, _setChannelButton;
};

#endif
