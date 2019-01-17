#ifndef TIMING_H
#define TIMING_H

class Timing
{
public:
	Timing(double timeInMS, unsigned timestepNumber, double beatValue, unsigned audioLevel) :
		_timeInMs(timeInMS),
		_timestepNumber(timestepNumber),
		_beatValue(beatValue),
		_audioLevel(audioLevel)
	{ }
	
	double TimeInMS() const { return _timeInMs; }
	double BeatValue() const { return _beatValue; }
	unsigned TimestepNumber() const { return _timestepNumber; }
	unsigned AudioLevel() const { return _audioLevel; }
	
private:
	double _timeInMs;
	unsigned _timestepNumber;
	double _beatValue;
	unsigned _audioLevel;
};

#endif
