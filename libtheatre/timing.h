#ifndef TIMING_H
#define TIMING_H

class Timing
{
public:
	Timing(double timeInMS, double beatValue, unsigned audioLevel) :
		_timeInMs(timeInMS),
		_beatValue(beatValue),
		_audioLevel(audioLevel)
	{ }
	
	double TimeInMS() const { return _timeInMs; }
	double BeatValue() const { return _beatValue; }
	unsigned AudioLevel() const { return _audioLevel; }
	
private:
	double _timeInMs;
	double _beatValue;
	unsigned _audioLevel;
};

#endif
