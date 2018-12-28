#ifndef TIMING_H
#define TIMING_H

class Timing
{
public:
	Timing(double timeInMS, double beatValue) : _timeInMs(timeInMS), _beatValue(beatValue)
	{ }
	
	double TimeInMS() const { return _timeInMs; }
	double BeatValue() const { return _beatValue; }
	
private:
	double _timeInMs;
	double _beatValue;
};

#endif
