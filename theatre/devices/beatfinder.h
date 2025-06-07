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

  BeatFinder(const std::string &device_name) : device_name_(device_name) {}

  ~BeatFinder() { close(); }

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
  unsigned _alsaPeriodSize = 256;
  unsigned _alsaBufferSize = 2048;
  std::unique_ptr<std::thread> _alsaThread;
  std::atomic<bool> _isStopping = false;
  bool _isOpen = false;
  std::string device_name_;
  std::mutex _mutex;
  struct Beat {
    Beat() = default;
    Beat(float c, float v) : value(v), confidence(c) {}
    float value = 0.0;
    float confidence = 0.0;
  };
  std::atomic<Beat> _beat = Beat();
  std::atomic<uint16_t> _audioLevel = 0;
  float _minimumConfidence = 0.05;

  void open();
  void close();
};

}  // namespace glight::theatre

#endif
