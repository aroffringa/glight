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

#include "components/inputselectmenu.h"
#include "components/objecttree.h"
#include "components/propertiesbox.h"

/**
	@author Andre Offringa
*/
class EffectsFrame : public Gtk::VPaned
{
	public:
		EffectsFrame(class Management& management, class ShowWindow& parentWindow);

private:
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
		void onInputSelected(class PresetValue* preset);
		void onChangeManagement(class Management &management)
		{
			_nameFrame.ChangeManagement(management);
			_management = &management;
		}

		Gtk::Frame _effectsFrame;
		ObjectTree _effectsList;
		Gtk::VBox _effectsVBox;
		Gtk::HBox _effectsHBox;
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
			Gtk::TreeModelColumn<size_t> _inputIndex;
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
		InputSelectMenu _controllablesMenu;
		std::unique_ptr<Gtk::Menu> _popupEffectMenu;
		std::vector<std::unique_ptr<Gtk::MenuItem>> _popupEffectMenuItems;
		AvoidRecursion _delayUpdates;
};

#endif

