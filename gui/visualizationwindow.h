#ifndef VISUALIZATIONWINDOW_H
#define VISUALIZATIONWINDOW_H

#include "../theatre/position.h"

#include <gtkmm/checkmenuitem.h>
#include <gtkmm/drawingarea.h>
#include <gtkmm/menu.h>
#include <gdkmm/pixbuf.h>
#include <gtkmm/separatormenuitem.h>
#include <gtkmm/window.h>

/**
	@author Andre Offringa
*/
class VisualizationWindow : public Gtk::Window {
public:
	VisualizationWindow(class Management* management, class EventTransmitter* eventTransmitter);
	~VisualizationWindow();

	void Update()
	{
		queue_draw();
	}
	
	void SetDryMode(class Management* dryManagement)
	{
		_dryManagement = dryManagement;
		Update();
	}
	void SetRealMode()
	{
		_dryManagement = nullptr;
		Update();
	}
	void MakeDryModeReal()
	{
		_management = _dryManagement;
		_dryManagement = nullptr;
		Update();
	}
private:
	Gtk::DrawingArea _drawingArea;
	Management* _management;
	Management* _dryManagement;
	class EventTransmitter* _eventTransmitter;
	bool _isInitialized, _isTimerRunning;
	sigc::connection _timeoutConnection;
	enum DragType {
		NotDragging,
		DragFixture,
		DragRectangle
	} _dragType;
	std::vector<class Fixture*> _selectedFixtures;
	Position _draggingStart, _draggingTo;

	Gtk::Menu _popupMenu;
	Gtk::SeparatorMenuItem _miSeparator1, _miSeparator2;
	Gtk::MenuItem _miAlignHorizontally, _miAlignVertically, _miDistributeEvenly, _miAdd, _miRemove;
	Gtk::CheckMenuItem _miFullscreen;
	
	void inializeContextMenu();
	void initialize();
	void drawAll(const Cairo::RefPtr< Cairo::Context>& cairo);
	void drawManagement(const Cairo::RefPtr< Cairo::Context>& cairo, class Management& management, size_t yOffset, size_t height);
	void onTheatreChanged();
	bool onButtonPress(GdkEventButton* event);
	bool onButtonRelease(GdkEventButton* event);
	bool onMotion(GdkEventMotion* event);
	bool onExpose(const Cairo::RefPtr<Cairo::Context>& context);
	bool onTimeout()
	{
		Update();
		return true;
	}
	
	void onAlignHorizontally();
	void onAlignVertically();
	void onDistributeEvenly();
	void onAddFixtures();
	void onRemoveFixtures();
	void onFullscreen();
	
	double scale(Management& management, double width, double height);
	double invScale(Management& management, double width, double height)
	{
		double sc = scale(management, width, height);
		if(sc == 0.0)
			return 1.0;
		else
			return 1.0 / sc;
	}
	class Fixture* fixtureAt(Management& management, const Position& position);
	void selectFixtures(const Position& a, const Position& b);
};

#endif
