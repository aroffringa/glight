#ifndef EFFECT_PROPERTIES_WINDOW_H
#define EFFECT_PROPERTIES_WINDOW_H

#include "propertieswindow.h"

#include "components/inputselectmenu.h"
#include "components/propertiesbox.h"

#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>

/**
	@author Andre Offringa
*/
class EffectPropertiesWindow : public PropertiesWindow {
public:
	EffectPropertiesWindow(class Effect& effect, class Management& management, class ShowWindow& parentWindow);
	
	FolderObject& GetObject() final override;
	
private:
	void fillProperties();
	void fillConnectionsList();

	bool onAddConnectionClicked(GdkEventButton* event);
	void onRemoveConnectionClicked();
	void onSelectedConnectionChanged();
	void onInputSelected(class PresetValue* preset);
	void onChangeManagement(class Management &management)
	{
		_management = &management;
	}
	void onUpdateControllables();

	Gtk::Label _titleLabel;
	Gtk::TreeView _connectionsListView;
	Glib::RefPtr<Gtk::ListStore> _connectionsListModel;
	struct ConnectionsListColumns : public Gtk::TreeModelColumnRecord
	{
		ConnectionsListColumns()
			{ add(_title); add(_index); add(_inputIndex); }
	
		Gtk::TreeModelColumn<Glib::ustring> _title;
		Gtk::TreeModelColumn<size_t> _index;
		Gtk::TreeModelColumn<size_t> _inputIndex;
	} _connectionsListColumns;
	
	Gtk::VBox _topBox;
	Gtk::HBox _mainHBox, _connectionsBox;
	Gtk::Frame _connectionsFrame, _propertiesFrame;
	std::unique_ptr<class PropertySet> _propertySet;
	PropertiesBox _propertiesBox;
	
	Gtk::ScrolledWindow _connectionsScrolledWindow;
	Gtk::VButtonBox _connectionsButtonBox;
	Gtk::Button _addConnectionButton, _removeConnectionButton;
	
	InputSelectMenu _controllablesMenu;
	
	Effect* _effect;
	Management* _management;
	ShowWindow& _parentWindow;
};

#endif
