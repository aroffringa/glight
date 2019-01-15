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
#include "components/propertiesbox.h"

/**
	@author Andre Offringa
*/
class EffectsFrame : public Gtk::VPaned
{
	public:
		EffectsFrame(class Management& management, class ShowWindow& parentWindow);

		void Update() { fillEffectsList(); }
		void UpdateAfterPresetRemoval() { Update(); }
		
		void ChangeManagement(class Management &management)
		{
			_nameFrame.ChangeManagement(management);
			_management = &management;
			Update();
		}
private:
		void fillEffectsList();
		void fillProperties(class Effect& effect);
		void fillConnectionsList(class Effect& effect);

		void initEffectsPart();
		void initPropertiesPart();
		
		Effect* getSelectedEffect();

		bool onNewEffectClicked(GdkEventButton* event);
		void onNewEffectMenuClicked(enum Effect::Type effectType);
		void onDeleteEffectClicked();
		void onSelectedEffectChanged();
		bool onAddConnectionClicked(GdkEventButton* event);
		void onRemoveConnectionClicked();
		void onSelectedConnectionChanged();
		void onNameChange();
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

		Gtk::VBox _effectsVBox;
		Gtk::HBox _effectsHBox;
		Gtk::Frame _effectsFrame;
		Gtk::ScrolledWindow _effectsScrolledWindow;
		Gtk::VButtonBox _effectsButtonBox;
		Gtk::Button _newEffectButton, _deleteEffectButton;
		NameFrame _nameFrame;

		Gtk::TreeView _connectionsListView;
		Glib::RefPtr<Gtk::ListStore> _connectionsListModel;
		struct ConnectionsListColumns : public Gtk::TreeModelColumnRecord
		{
			ConnectionsListColumns()
				{ add(_title); add(_index); }
		
			Gtk::TreeModelColumn<Glib::ustring> _title;
			Gtk::TreeModelColumn<size_t> _index;
		} _connectionsListColumns;
		
		Gtk::HBox _propertiesHBox, _connectionsBox;
		Gtk::Frame _connectionsFrame, _propertiesFrame;
		std::unique_ptr<class PropertySet> _propertySet;
		PropertiesBox _propertiesBox;
		
		Gtk::ScrolledWindow _connectionsScrolledWindow;
		Gtk::VButtonBox _connectionsButtonBox;
		Gtk::Button _addConnectionButton, _removeConnectionButton;

		Management* _management;
		class ShowWindow& _parentWindow;
		ControllableSelectMenu _controllablesMenu;
		std::unique_ptr<Gtk::Menu> _popupEffectMenu;
		std::vector<std::unique_ptr<Gtk::MenuItem>> _popupEffectMenuItems;
		AvoidRecursion _delayUpdates;
};

#endif

