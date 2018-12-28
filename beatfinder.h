#ifndef BEATFINDER_H
#define BEATFINDER_H

#include <alsa/asoundlib.h>

#include <string>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <thread>

/**
	@author Andre Offringa
*/
class BeatFinder {
public:
	class AlsaError : public std::runtime_error
	{
		public:
			AlsaError(const std::string &message) : runtime_error(std::string("Alsa error: ") + message)
			{ }
	};

	BeatFinder() : _alsaPeriodSize(256), _alsaBufferSize(2048), _alsaThread(), _isStopping(false), _isOpen(false), _minimumConfidence(0.05)
	{
	}

	virtual ~BeatFinder()
	{
		setStopping();
		close();
	}

	void Start() {
		AlsaThread alsaThreadFunc(*this);
		_alsaThread.reset(new std::thread(alsaThreadFunc));
	}
	
	void GetBeatValue(double& beatValue, double& confidence)
	{
		std::unique_lock<std::mutex> lock(_mutex);
		beatValue = _beatValue;
		confidence = _confidence;
	}
private:
	struct AlsaThread {
		public:
			BeatFinder &_player;
			AlsaThread(BeatFinder &player) : _player(player) { }
			void operator()()
			{
				try {
					_player.open();
					std::cout << "Alsa thread finished.\n";
				}
				catch(std::exception& e) {
					std::cout << "Could not open alsa device: beat finder is not working.\n";
				}
			}
	};
	
	snd_pcm_t *_handle;
	unsigned _alsaPeriodSize, _alsaBufferSize;
	std::unique_ptr<std::thread> _alsaThread;
	bool _isStopping;
	bool _isOpen;
	std::mutex _mutex;
	double _confidence, _minimumConfidence;
	double _beatValue;

	void open();
	void close();
	bool isStopping()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		return _isStopping;
	}
	void setStopping()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		_isStopping = true;
	}
};

#endif
