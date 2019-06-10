#ifndef VISUALIZATIONWINDOW_H
#define VISUALIZATIONWINDOW_H

#include "../theatre/position.h"

#include <gtkmm/drawingarea.h>
#include <gtkmm/window.h>

#include <gdkmm/pixbuf.h>

/**
	@author Andre Offringa
*/
class VisualizationWindow : public Gtk::Window {
public:
	VisualizationWindow(class Management* management);
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
	bool _isInitialized, _isTimerRunning;
	sigc::connection _timeoutConnection;
	class Fixture* _draggingFixture;
	Position _draggingStart;

	void initialize();
	void drawAll(const Cairo::RefPtr< Cairo::Context>& cairo);
	void drawManagement(const Cairo::RefPtr< Cairo::Context>& cairo, class Management& management, size_t yOffset, size_t height);
	bool onButtonPress(GdkEventButton* event);
	bool onButtonRelease(GdkEventButton* event);
	bool onMotion(GdkEventMotion* event);
	bool onExpose(const Cairo::RefPtr<Cairo::Context>& context);
	bool onTimeout()
	{
		Update();
		return true;
	}
	double scale(Management& management, double width, double height);
	double invScale(Management& management, double width, double height)
	{
		double sc = scale(management, width, height);
		if(sc == 0.0)
			return 0.0;
		else
			return 1.0 / sc;
	}
	class Fixture* fixtureAt(Management& management, const Position& position);
};

#endif
