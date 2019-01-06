#ifndef EFFECTS_FRAME_H
#define EFFECTS_FRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/liststore.h>
#include <gtkmm/paned.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>

#include "avoidrecursion.h"
#include "nameframe.h"

#include "components/controllableselectmenu.h"

/**
	@author Andre Offringa
*/
class EffectsFrame : public Gtk::VPaned
{
	public:
		EffectsFrame(class Management& management, class ShowWindow& parentWindow);

		void Update() { fillEffectsList(); }
		void UpdateAfterPresetRemoval() { fillEffectsList(); }
		
		void ChangeManagement(class Management &management)
		{
			_nameFrame.ChangeManagement(management);
			_management = &management;
			fillEffectsList();
		}
private:
		void fillEffectsList();

		void initEffectsPart();
		void initPropertiesPart();

		void onNewEffectClicked();
		void onDeleteEffectClicked();
		void onSelectedEffectChanged();
		bool onAddConnectionClicked(GdkEventButton* event);
		void onRemoveConnectionClicked();
		void onNameChange() { fillEffectsList(); }
		void onControllableSelected(class PresetValue* preset);

		Gtk::TreeView _effectsListView;
		Glib::RefPtr<Gtk::ListStore> _effectsListModel;
		struct EffectsListColumns : public Gtk::TreeModelColumnRecord
		{
			EffectsListColumns()
				{ add(_title); add(_effect); }
		
			Gtk::TreeModelColumn<Glib::ustring> _title;
			Gtk::TreeModelColumn<class Effect *> _effect;
		} _effectsListColumns;

		Gtk::TreeView _connectionsListView;
		Glib::RefPtr<Gtk::ListStore> _connectionsListModel;
		struct ConnectionsListColumns : public Gtk::TreeModelColumnRecord
		{
			ConnectionsListColumns()
				{ add(_title); add(_index); }
		
			Gtk::TreeModelColumn<Glib::ustring> _title;
			Gtk::TreeModelColumn<size_t> _index;
		} _connectionsListColumns;

		Gtk::VBox _effectsVBox;
		Gtk::HBox _effectsHBox, _connectionsBox;
		Gtk::Frame _effectsFrame;
		Gtk::Frame _connectionsFrame;

		Gtk::ScrolledWindow _effectsScrolledWindow, _connectionsScrolledWindow;

		Gtk::VButtonBox _effectsButtonBox, _connectionsButtonBox;
		Gtk::Button _newEffectButton, _deleteEffectButton;
		Gtk::Button _addConnectionButton, _removeConnectionButton;

		Management* _management;
		class ShowWindow& _parentWindow;
		NameFrame _nameFrame;
		ControllableSelectMenu _controllablesMenu;
		AvoidRecursion _delayUpdates;
};

#endif

