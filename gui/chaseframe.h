#ifndef CHASEFRAME_H
#define CHASEFRAME_H

#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/liststore.h>
#include <gtkmm/paned.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/scale.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/treeview.h>

#include "avoidrecursion.h"

/**
	@author Andre Offringa
*/
class ChaseFrame : public Gtk::VPaned {
	public:
		ChaseFrame(class Management& management, class ShowWindow& parentWindow);
		~ChaseFrame();

		void Update() { fillChaseList(); }
		void UpdateAfterPresetRemoval() { fillChaseList(); }
		
		void ChangeManagement(class Management& management)
		{
			_management = &management;
			fillChaseList();
		}
		
	private:
		void fillChaseList();
		void initUpperPanel();
		void initLowerPanel();
		void onSelectedChaseChanged();
		void onTriggerTypeChanged();
		void onTriggerSpeedChanged();
		void onTransitionSpeedChanged();
		void onSyncCountChanged();
		void onBeatSpeedChanged();
		void onDeleteChaseClicked();
		
		class Chase* getSelectedChase();

		struct ChaseListColumns : public Gtk::TreeModelColumnRecord
		{
			ChaseListColumns()
				{ add(_title); add(_chase); }
		
			Gtk::TreeModelColumn<Glib::ustring> _title;
			Gtk::TreeModelColumn<class Chase *> _chase;
		} _chaseListColumns;
		Gtk::Frame _upperFrame;
		Gtk::TreeView _chaseListView;
		Glib::RefPtr<Gtk::ListStore> _chaseListModel;
		Gtk::HBox _upperBox;
		Gtk::ScrolledWindow _chaseScrolledWindow;
		Gtk::VButtonBox _upperButtonBox;
		Gtk::Button _deleteChaseButton;
		
		Gtk::Frame _bottomFrame;
		Gtk::VBox _bottomBox;
		
		Gtk::RadioButton _delayTriggerCheckButton;
		Gtk::Label _triggerSpeedLabel;
		Gtk::HScale _triggerSpeed;
		Gtk::Label _transitionSpeedLabel;
		Gtk::HScale _transitionSpeed;
		
		Gtk::RadioButton _synchronizedTriggerCheckButton;
		Gtk::Label _synchronizationsLabel;
		Gtk::HScale _synchronizationsCount;
		
		Gtk::RadioButton _beatTriggerCheckButton;
		Gtk::Label _beatSpeedLabel;
		Gtk::HScale _beatSpeed;

		AvoidRecursion _delayUpdates;
		
		Management* _management;
		ShowWindow& _parentWindow;
};

#endif
