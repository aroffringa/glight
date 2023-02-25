#ifndef THEATRE_BEAT_FINDER_H_
#define THEATRE_BEAT_FINDER_H_

#include <alsa/asoundlib.h>

#include <atomic>
#include <iostream>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>

namespace glight::theatre {

class BeatFinder {
 public:
  class AlsaError : public std::runtime_error {
   public:
    AlsaError(const std::string &message)
        : runtime_error(std::string("Alsa error: ") + message) {}
  };

  BeatFinder()
      : _alsaPeriodSize(256),
        _alsaBufferSize(2048),
        _alsaThread(),
        _isStopping(false),
        _isOpen(false),
        _beat(Beat()),
        _audioLevel(0),
        _minimumConfidence(0.05) {}

  virtual ~BeatFinder() { close(); }

  void Start() {
    _alsaThread = std::make_unique<std::thread>([&]() { TryOpen(); });
  }

  void GetBeatValue(double &beatValue, double &confidence) {
    const Beat beat = _beat;
    beatValue = beat.value;
    confidence = beat.confidence;
  }

  uint16_t GetAudioLevel() const { return _audioLevel; }

 private:
  void TryOpen() {
    try {
      open();
    } catch (std::exception &e) {
      std::cout << "Could not open alsa device: beat finder is not working.\n";
    }
  }

  snd_pcm_t *_handle;
  unsigned _alsaPeriodSize, _alsaBufferSize;
  std::unique_ptr<std::thread> _alsaThread;
  std::atomic<bool> _isStopping;
  bool _isOpen;
  std::mutex _mutex;
  struct Beat {
    Beat() = default;
    Beat(float c, float v) : value(v), confidence(c) {}
    float value = 0.0;
    float confidence = 0.0;
  };
  std::atomic<Beat> _beat;
  uint32_t _audioLevelAccumulator;
  std::atomic<uint16_t> _audioLevel;
  float _minimumConfidence;

  void open();
  void close();
};

}  // namespace glight::theatre

#endif
