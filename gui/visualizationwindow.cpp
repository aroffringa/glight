#include "visualizationwindow.h"

#include "../libtheatre/management.h"
#include "../libtheatre/dmxdevice.h"
#include "../libtheatre/fixture.h"
#include "../libtheatre/theatre.h"
#include "../libtheatre/valuesnapshot.h"

#include <glibmm/main.h>

VisualizationWindow::VisualizationWindow(Management* management) :
	_management(management),
	_dryManagement(nullptr),
	_isInitialized(false), _isTimerRunning(false),
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
	const ValueSnapshot snapshot = management.Snapshot();
	Position extend = theatre.Extend();
	if(extend.X() == 0.0 || extend.Y() == 0.0)
		return 0.0;
	else
		return std::min(width/extend.X(), height/extend.Y());
}

void VisualizationWindow::drawManagement(const Cairo::RefPtr< Cairo::Context>& cairo, Management& management, size_t yOffset, size_t height)
{
	const ValueSnapshot snapshot = management.Snapshot();
	const std::vector<std::unique_ptr<Fixture>>& fixtures = management.Theatre().Fixtures();
	if(!fixtures.empty())
	{
		cairo->save();
		double sc = scale(management, _drawingArea.get_width(), height);
		cairo->scale(sc, sc);
		for(const std::unique_ptr<Fixture>& f : fixtures)
		{
			Color c = f->GetColor(snapshot);

			cairo->set_source_rgb((double) c.Red()/224.0+0.125, (double) c.Green()/224.0+0.125, (double) c.Blue()/224.0+0.125);

			double x = f->Position().X() + 0.5;
			double y = f->Position().Y() + 0.5;
			cairo->arc(x, y + yOffset, 0.4, 0.0, 2.0 * M_PI);
			cairo->fill();
		}
		cairo->restore();
	}
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
	if(event->button == 1)
	{
		double sc = invScale(*_management, _drawingArea.get_width(), _drawingArea.get_height());
		_draggingStart = Position(event->x, event->y) * sc;
		_draggingFixture = fixtureAt(*_management, _draggingStart);
		queue_draw();
	}
	return true;
}

bool VisualizationWindow::onButtonRelease(GdkEventButton* event)
{
	if(event->button == 1)
	{
		double sc = invScale(*_management, _drawingArea.get_width(), _drawingArea.get_height());
		_draggingStart = Position(event->x, event->y) * sc;
		_draggingFixture = nullptr;
		queue_draw();
	}
	return true;
}

bool VisualizationWindow::onMotion(GdkEventMotion* event)
{
	if(_draggingFixture)
	{
		double width = _drawingArea.get_width();
		double height = _drawingArea.get_height();
		Position extend = _management->Theatre().Extend();
		double scale = std::min(width/extend.X(), height/extend.Y());
		Position pos = Position(event->x, event->y) / scale;
		_draggingFixture->Position() += pos - _draggingStart;
		_draggingStart = pos;
		queue_draw();
	}
	return true;
}
