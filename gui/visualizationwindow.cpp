#include "eventtransmitter.h"
#include "visualizationwindow.h"

#include "../theatre/management.h"
#include "../theatre/dmxdevice.h"
#include "../theatre/fixture.h"
#include "../theatre/theatre.h"
#include "../theatre/valuesnapshot.h"

#include <glibmm/main.h>

VisualizationWindow::VisualizationWindow(Management* management, EventTransmitter* eventTransmitter) :
	_management(management),
	_dryManagement(nullptr),
	_eventTransmitter(eventTransmitter),
	_isInitialized(false), _isTimerRunning(false),
	_dragType(NotDragging),
	_selectedFixtures(),
	_miAlignHorizontally("Align horizontally"),
	_miAlignVertically("Align vertically"),
	_miDistributeEvenly("Distribute evenly"),
	_miRemove("Remove")
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
	inializeContextMenu();
}

VisualizationWindow::~VisualizationWindow()
{
	_timeoutConnection.disconnect();
}

void VisualizationWindow::inializeContextMenu()
{
	_miAlignHorizontally.signal_activate().connect([&]{ onAlignHorizontally(); });
	_popupMenu.add(_miAlignHorizontally);
	
	_miAlignVertically.signal_activate().connect([&]{ onAlignVertically(); });
	_popupMenu.add(_miAlignVertically);
	
	_miDistributeEvenly.signal_activate().connect([&]{ onDistributeEvenly(); });
	_popupMenu.add(_miDistributeEvenly);
	
	_popupMenu.add(_miSeparator1);
	
	_miRemove.signal_activate().connect([&]{ onRemoveFixtures(); });
	_popupMenu.add(_miRemove);
	
	_popupMenu.show_all_children();
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

void VisualizationWindow::onTheatreChanged()
{
	for(size_t i = _selectedFixtures.size(); i != 0; --i)
	{
		if(!_management->Theatre().Contains(*_selectedFixtures[i-1]))
			_selectedFixtures.erase(_selectedFixtures.begin() + i-1);
	}
	Update();
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
		size_t shapeCount = f->Type().ShapeCount();
		for(size_t i=0; i!=shapeCount; ++i)
		{
			size_t shapeIndex = shapeCount - i - 1;
			Color c = f->GetColor(snapshot, shapeIndex);

			cairo->set_source_rgb((double) c.Red()/224.0+0.125, (double) c.Green()/224.0+0.125, (double) c.Blue()/224.0+0.125);

			double radius = shapeCount==1 ? 0.4 : 0.33 + 0.07 * double(shapeIndex) / (shapeCount - 1);
			double x = f->Position().X() + 0.5;
			double y = f->Position().Y() + 0.5;
			cairo->arc(x, y + yOffset/sc, radius, 0.0, 2.0 * M_PI);
			cairo->fill();
		}
	}
	cairo->set_source_rgb(0.2, 0.2, 1.0);
	cairo->set_line_width(4.0/sc);
	for(const Fixture* f : _selectedFixtures)
	{
		double x = f->Position().X() + 0.5;
		double y = f->Position().Y() + 0.5;
		cairo->arc(x, y + yOffset/sc, 0.4, 0.0, 2.0 * M_PI);
		cairo->stroke();
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
	if(!_dryManagement && (event->button==1 || event->button==3))
	{
		double sc = invScale(*_management, _drawingArea.get_width(), _drawingArea.get_height());
		Position pos = Position(event->x, event->y) * sc;
		Fixture* selectedFixture = fixtureAt(*_management, pos);
		if(selectedFixture)
		{
			// Was a fixture clicked that was already selected? Then keep all selected.
			// If not, select the clicked fixture:
			if(std::find(_selectedFixtures.begin(), _selectedFixtures.end(), selectedFixture)
				== _selectedFixtures.end())
			{
				_selectedFixtures.assign(1, selectedFixture);
			}
		}
		else {
			_selectedFixtures.clear();
		}
		
		if(event->button == 1)
		{
			_draggingStart = pos;
			if(!_selectedFixtures.empty())
			{
				_dragType = DragFixture;
			}
			else {
				_dragType = DragRectangle;
				_draggingTo = pos;
			}
			queue_draw();
		}
		else if(event->button == 3)
		{
			queue_draw();
			_miAlignHorizontally.set_sensitive(_selectedFixtures.size() >= 2);
			_miAlignVertically.set_sensitive(_selectedFixtures.size() >= 2);
			_miDistributeEvenly.set_sensitive(_selectedFixtures.size() >= 2);
			_miRemove.set_sensitive(_selectedFixtures.size() >= 1);
			_popupMenu.popup(event->button, event->time);
		}
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
			for(Fixture* fixture : _selectedFixtures)
				fixture->Position() += pos - _draggingStart;
			_draggingStart = pos;
		}
		else {
			_draggingTo = pos;
			selectFixtures(_draggingStart, _draggingTo);
		}
		queue_draw();
	}
	return true;
}

void VisualizationWindow::selectFixtures(const Position& a, const Position& b)
{
	_selectedFixtures.clear();
	double
		x1 = a.X(), y1 = a.Y(),
		x2 = b.X(), y2 = b.Y();
	if(x1 > x2) std::swap(x1, x2);
	if(y1 > y2) std::swap(y1, y2);
	Position first(x1 - 0.1, y1 - 0.1);
	Position second(x2 - 0.9, y2 - 0.9);
	if(second.X() - first.X() > 0.0 && second.Y() - first.Y() > 0.0)
	{
		const std::vector<std::unique_ptr<Fixture>>& fixtures = _management->Theatre().Fixtures();
		for(const std::unique_ptr<Fixture>& fixture : fixtures)
		{
			if(fixture->Position().InsideRectangle(first, second))
				_selectedFixtures.emplace_back(fixture.get());
		}
	}
}

void VisualizationWindow::onAlignHorizontally()
{
	if(_selectedFixtures.size() >= 2)
	{
		double y = 0.0;
		
		for(const Fixture* fixture :_selectedFixtures)
			y += fixture->Position().Y();
		
		y /= _selectedFixtures.size();
		
		for(Fixture* fixture :_selectedFixtures)
			fixture->Position().Y() = y;
	}
}

void VisualizationWindow::onAlignVertically()
{
	if(_selectedFixtures.size() >= 2)
	{
		double x = 0.0;
		
		for(const Fixture* fixture :_selectedFixtures)
			x += fixture->Position().X();
		
		x /= _selectedFixtures.size();
		
		for(Fixture* fixture :_selectedFixtures)
			fixture->Position().X() = x;
	}
}

void VisualizationWindow::onDistributeEvenly()
{
	if(_selectedFixtures.size() >= 2)
	{
		double left = _selectedFixtures[0]->Position().X();
		double right = _selectedFixtures[0]->Position().X();
		double top = _selectedFixtures[0]->Position().Y();
		double bottom = _selectedFixtures[0]->Position().Y();
		
		for(size_t i=1; i!=_selectedFixtures.size(); ++i)
		{
			const Fixture* fixture = _selectedFixtures[i];
			left = std::min(fixture->Position().X(), left);
			right = std::max(fixture->Position().X(), right);
			top = std::min(fixture->Position().Y(), top);
			bottom = std::max(fixture->Position().Y(), bottom);
		}
		
		std::vector<Fixture*> list = _selectedFixtures;
		if(left == right)
		{
			std::sort(list.begin(), list.end(), [](const Fixture* a, const Fixture* b)
				{ return a->Position().Y() < b->Position().Y(); } );
			for(size_t i=0; i!=list.size(); ++i)
			{
				double y = double(i) / double(list.size()-1) * (bottom - top) + top;
				list[i]->Position().Y() = y;
			}
		}
		else {
			std::sort(list.begin(), list.end(), [](const Fixture* a, const Fixture* b)
				{ return a->Position().X() < b->Position().X(); } );
			left = list.front()->Position().X();
			right = list.back()->Position().X();
			top = list.front()->Position().Y();
			bottom = list.back()->Position().Y();
			for(size_t i=0; i!=list.size(); ++i)
			{
				double r = double(i) / double(list.size()-1);
				double x = r * (right - left) + left;
				double y = r * (bottom - top) + top;
				list[i]->Position() = Position(x, y);
			}
		}
	}
}

void VisualizationWindow::onRemoveFixtures()
{
	for(Fixture* fixture : _selectedFixtures)
	{
		_management->RemoveFixture(*fixture);
	}
	_selectedFixtures.clear();
	_eventTransmitter->EmitUpdate();
}
