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

#include "components/objecttree.h"

/**
	@author Andre Offringa
*/
class ChaseFrame : public Gtk::VPaned {
	public:
		ChaseFrame(class Management& management, class ShowWindow& parentWindow);
		~ChaseFrame();

		void ChangeManagement(class Management& management)
		{
			_management = &management;
		}
		
		void Select(const class Chase& chase);
		
	private:
		void initUpperPanel();
		void initLowerPanel();
		void onSelectedChaseChanged();
		void onTriggerTypeChanged();
		void onTriggerSpeedChanged();
		void onTransitionSpeedChanged();
		void onTransitionTypeChanged();
		void onSyncCountChanged();
		void onBeatSpeedChanged();
		void onDeleteChaseClicked();
		
		class Chase* getSelectedChase();

		Gtk::Frame _upperFrame;
		ObjectTree _chaseList;
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
		Gtk::HBox _transitionTypeBox;
		Gtk::Label _transitionTypeLabel;
		Gtk::RadioButton _transitionNoneRB, _transitionFadeRB, _transitionFadeThroughBlackRB, _transitionErraticRB;
		
		Gtk::RadioButton _synchronizedTriggerCheckButton;
		Gtk::Label _synchronizationsLabel;
		Gtk::HScale _synchronizationsCount;
		
		Gtk::RadioButton _beatTriggerCheckButton;
		Gtk::Label _beatSpeedLabel;
		Gtk::HScale _beatSpeed;
		
		Management* _management;
		ShowWindow& _parentWindow;
};

#endif
