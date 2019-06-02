#ifndef POSITION_H
#define POSITION_H

class Position
{
public:
	Position() : _x(0.0), _y(0.0)
	{ }
	
	Position(double x, double y) : _x(x), _y(y)
	{ }
	
	double& X() { return _x; }
	const double& X() const { return _x; }
	
	double& Y() { return _y; }
	const double& Y() const { return _y; }
	
	bool operator==(const Position& rhs) const
	{ return p() == rhs.p(); }
	
	bool operator<(const Position& rhs) const
	{ return p() < rhs.p(); }
	
private:
	std::pair<double, double> p() const { return std::make_pair(_x, _y); }
	
	double _x, _y;
};

#endif
