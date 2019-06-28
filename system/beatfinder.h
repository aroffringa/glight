#ifndef BEATFINDER_H
#define BEATFINDER_H

#include <alsa/asoundlib.h>

#include <atomic>
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

	BeatFinder() : 
		_alsaPeriodSize(256), 
		_alsaBufferSize(2048),
		_alsaThread(),
		_isStopping(false),
		_isOpen(false),
		_beat(Beat()),
		_audioLevel(0),
		_minimumConfidence(0.05)
	{
	}

	virtual ~BeatFinder()
	{
		close();
	}

	void Start() {
		AlsaThread alsaThreadFunc(*this);
		_alsaThread.reset(new std::thread(alsaThreadFunc));
	}
	
	void GetBeatValue(double& beatValue, double& confidence)
	{
		Beat beat = _beat;
		beatValue = beat.value;
		confidence = beat.confidence;
	}
	
	uint16_t GetAudioLevel() const
	{
		return _audioLevel;
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
				}
				catch(std::exception& e) {
					std::cout << "Could not open alsa device: beat finder is not working.\n";
				}
			}
	};
	
	snd_pcm_t *_handle;
	unsigned _alsaPeriodSize, _alsaBufferSize;
	std::unique_ptr<std::thread> _alsaThread;
	std::atomic<bool> _isStopping;
	bool _isOpen;
	std::mutex _mutex;
	struct Beat {
		Beat() : value(0.0), confidence(0.0) { }
		Beat(float c, float v) : value(v), confidence(c) { }
		float value;
		float confidence;
	};
	std::atomic<Beat> _beat;
	std::atomic<uint16_t> _audioLevel;
	float _minimumConfidence;

	void open();
	void close();
};

#endif
