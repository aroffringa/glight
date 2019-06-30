#include "visualizationwindow.h"

#include "../theatre/management.h"
#include "../theatre/dmxdevice.h"
#include "../theatre/fixture.h"
#include "../theatre/theatre.h"
#include "../theatre/valuesnapshot.h"

#include <glibmm/main.h>

VisualizationWindow::VisualizationWindow(Management* management) :
	_management(management),
	_dryManagement(nullptr),
	_isInitialized(false), _isTimerRunning(false),
	_dragType(NotDragging),
	_draggingFixture(nullptr)
{
	set_title("Glight - visualization");
	set_default_size(600, 200);

	_drawingArea.set_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
	_drawingArea.signal_draw().connect(sigc::mem_fun(*this, &VisualizationWindow::onExpose));
	_drawingArea.signal_button_press_event().connect(sigc::mem_fun(*this, &VisualizationWindow::onButtonPress));
	_drawingArea.signal_button_release_event().connect(sigc::mem_fun(*this, &VisualizationWindow::onButtonRelease));
	_drawingArea.signal_motion_notify_event().connect(sigc::mem_fun(*this, &VisualizationWindow::onMotion));
	add(_drawingArea);
	_drawingArea.show();
}

VisualizationWindow::~VisualizationWindow()
{
	_timeoutConnection.disconnect();
}

void VisualizationWindow::initialize()
{
	queue_draw();
	_isInitialized = true;

	if(!_isTimerRunning)
	{
		_timeoutConnection = Glib::signal_timeout().connect( sigc::mem_fun(*this, &VisualizationWindow::onTimeout), 40);
		_isTimerRunning = true;
	}
}

bool VisualizationWindow::onExpose(const Cairo::RefPtr<Cairo::Context>& context)
{
	if(!_isInitialized)
		initialize();

	drawAll(context);
	return true;
}

double VisualizationWindow::scale(Management& management, double width, double height)
{
	const Theatre &theatre = management.Theatre();
	Position extend = theatre.Extend();
	if(extend.X() == 0.0 || extend.Y() == 0.0)
		return 1.0;
	else
		return std::min(width/extend.X(), height/extend.Y());
}

void VisualizationWindow::drawManagement(const Cairo::RefPtr< Cairo::Context>& cairo, Management& management, size_t yOffset, size_t height)
{
	const ValueSnapshot snapshot = management.Snapshot();
	const std::vector<std::unique_ptr<Fixture>>& fixtures = management.Theatre().Fixtures();
	cairo->save();
	double sc = scale(management, _drawingArea.get_width(), height);
	cairo->scale(sc, sc);
	for(const std::unique_ptr<Fixture>& f : fixtures)
	{
		Color c = f->GetColor(snapshot);

		cairo->set_source_rgb((double) c.Red()/224.0+0.125, (double) c.Green()/224.0+0.125, (double) c.Blue()/224.0+0.125);

		double x = f->Position().X() + 0.5;
		double y = f->Position().Y() + 0.5;
		cairo->arc(x, y + yOffset/sc, 0.4, 0.0, 2.0 * M_PI);
		cairo->fill();
	}
	
	if(_dragType == DragRectangle)
	{
		std::pair<double, double> size = _draggingTo - _draggingStart;
		cairo->rectangle(_draggingStart.X(), _draggingStart.Y(), size.first, size.second);
		cairo->set_source_rgba(0.2, 0.2, 1.0, 0.5);
		cairo->fill_preserve();
		cairo->set_source_rgba(0.5, 0.5, 1.0, 0.8);
		cairo->set_line_width(2.0/sc);
		cairo->stroke();
	}
	
	cairo->restore();
}

void VisualizationWindow::drawAll(const Cairo::RefPtr< Cairo::Context>& cairo)
{
	double
		width = _drawingArea.get_width(),
		height = _drawingArea.get_height();

	cairo->set_source_rgba(0,0,0,1);
	cairo->rectangle(0, 0, width, height);
	cairo->fill();
	
	if(_dryManagement==nullptr)
	{
		drawManagement(cairo, *_management, 0, height);
	}
	else {
		drawManagement(cairo, *_management, 0, height/2.0);
		drawManagement(cairo, *_dryManagement, height/2.0, height/2.0);
	}
}

Fixture* VisualizationWindow::fixtureAt(Management& management, const Position& position)
{
	const std::vector<std::unique_ptr<Fixture>>& fixtures = management.Theatre().Fixtures();

	Fixture* fix = nullptr;
	double closest = std::numeric_limits<double>::max();
	for(const std::unique_ptr<Fixture>& f : fixtures)
	{
		if(position.InsideRectangle(f->Position(), f->Position().Add(1.0, 1.0)))
		{
			double distanceSq = position.SquaredDistance(f->Position().Add(0.5, 0.5));
			if(distanceSq <= 0.16 && distanceSq < closest)
			{
				fix = f.get();
				closest = distanceSq;
			}
		}
	}
	return fix;
}

bool VisualizationWindow::onButtonPress(GdkEventButton* event)
{
	if(event->button == 1 && !_dryManagement)
	{
		double sc = invScale(*_management, _drawingArea.get_width(), _drawingArea.get_height());
		_draggingStart = Position(event->x, event->y) * sc;
		_draggingFixture = fixtureAt(*_management, _draggingStart);
		if(_draggingFixture)
			_dragType = DragFixture;
		else {
			_dragType = DragRectangle;
			_draggingTo = _draggingStart;
		}
		queue_draw();
	}
	return true;
}

bool VisualizationWindow::onButtonRelease(GdkEventButton* event)
{
	if(event->button == 1 && !_dryManagement)
	{
		double sc = invScale(*_management, _drawingArea.get_width(), _drawingArea.get_height());
		if(_dragType == DragFixture)
		{
			_draggingStart = Position(event->x, event->y) * sc;
			_draggingFixture = nullptr;
		}
		else if(_dragType == DragRectangle)
		{
		}
		_dragType = NotDragging;
		queue_draw();
	}
	return true;
}

bool VisualizationWindow::onMotion(GdkEventMotion* event)
{
	if(_dragType!=NotDragging && !_dryManagement)
	{
		double width = _drawingArea.get_width();
		double height = _drawingArea.get_height();
		Position pos = Position(event->x, event->y) / scale(*_management, width, height);
		if(_dragType == DragFixture)
		{
			_draggingFixture->Position() += pos - _draggingStart;
			_draggingStart = pos;
		}
		else {
			_draggingTo = pos;
		}
		queue_draw();
	}
	return true;
}
